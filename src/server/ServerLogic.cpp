

#include <optional>

#include <websocketpp/frame.hpp>

#include <game.pb.h>
#include <server/Player.h>
#include <server/Server.h>

IPlayer const *ServerLogic::addPlayer(std::string const &userName,
                                      std::string const &displayName) {
    std::shared_ptr<IPlayer> player = std::make_shared<Player>(
        Player::Params{.username = userName, .displayName = displayName});
    auto [iter, success] = m_players.insert({player.get(), std::move(player)});
    return iter->first;
}

bool ServerLogic::removePlayer(IPlayer *player) { return bool(m_players.erase(player)); }

std::optional<std::shared_ptr<IPlayer>>
ServerLogic::findPlayer(std::string const &userName, std::string const &displayName) {
    auto iter = std::find_if(m_players.begin(), m_players.end(), [&](auto const &pair) {
        return pair.first->getUsername() == userName &&
               pair.first->getDisplayName() == displayName;
    });
    if (iter == m_players.end()) {
        return std::nullopt;
    }
    return iter->second;
}

GameInstance const *ServerLogic::createGameInstance(IPlayer *player1, IPlayer *player2) {
    auto game =
        std::make_shared<GameInstance>(GameInstance{.player1 = player1, .player2 = player2});

    auto mappedItem = std::make_pair(game.get(), std::move(game));

    auto [iter1, success1] = activeGames[player1->getConnection()].insert(mappedItem);
    auto [iter2, success2] = activeGames[player2->getConnection()].insert(mappedItem);

    return mappedItem.first;
}

bool ServerLogic::removeGameInstance(GameInstance *game) {
    auto hdl1 = game->player1->getConnection();
    auto hdl2 = game->player2->getConnection();
    return bool(activeGames[hdl1].erase(game)) && bool(activeGames[hdl1].erase(game));
}

void ServerLogic::sendErrorResponse(std::string const &error, ConnectionPtr hdl) {

    auto sharedConnectionPtr = m_server->get_con_from_hdl(hdl);
    if (sharedConnectionPtr) {
        sharedConnectionPtr->send(
            error.c_str(), error.size(), websocketpp::frame::opcode::value::text);
    } else {
        std::cerr << "Could not send response after invalid request: client disconnected."
                  << std::endl;
    }
}

void ServerLogic::decodeAndProcessRequest(ConnectionPtr hdl, Server::message_ptr msg) {

    std::string payload = msg->get_payload();

    game_proto::Request request;
    if (!request.ParseFromString(payload)) {

        game_proto::Response response;
        response.mutable_registration_response()->set_success(false);
        response.mutable_registration_response()->set_error(
            "invalid request, failed to parse request.");
        auto serializedResponse = response.SerializeAsString();

        auto sharedConnectionPtr = m_server->get_con_from_hdl(hdl);
        if (sharedConnectionPtr) {
            sharedConnectionPtr->send(serializedResponse.c_str(),
                                      serializedResponse.size(),
                                      websocketpp::frame::opcode::value::text);
        }
    }
}