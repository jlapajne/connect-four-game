#include <cstdint>
#include <format>
#include <iostream>

#include <client/Bot.h>
#include <client/IBot.h>
#include <server/RandomUtils.h>

RandomBot::RandomBot(Params p) : BotBase(std::move(p)) {}

void RandomBot::sendMoveRequest(GameId const &gameId, std::uint32_t columnIdx) {

    game_proto::Request request;
    auto &moveRequest = *request.mutable_move_request();
    moveRequest.set_game_id(gameId);
    moveRequest.set_column_idx(columnIdx);

    sendProtoMessage(request);
}
void RandomBot::sendFirstMoveRequest(GameId const &gameId) {
    sendMoveRequest(gameId, getRandomInt(ConnectFourGame::ColumnCount - 1));
}

void RandomBot::processAvailableMovesResponse(
    game_proto::AvailableMovesResponse const &response) {

    auto &availableMoves = response.column_idx();
    auto selectedColumnIdx = availableMoves[getRandomInt(availableMoves.size() - 1)];

    sendMoveRequest(response.game_id(), selectedColumnIdx);
}

std::shared_ptr<IBot> makeNewBot(BotType type,
                                 std::string name,
                                 ConnectionMetadata metadata,
                                 std::shared_ptr<Client> endpoint) {
    if (type == BotType::Random) {
        return std::make_shared<RandomBot>(RandomBot::Params{
            .name = std::move(name), .metadata = std::move(metadata), .endpoint = endpoint});
    }

    throw std::runtime_error("Unknown bot type.");
};
