

#include <iterator>
#include <mutex>
#include <optional>
#include <random>

#include <websocketpp/frame.hpp>

#include <game.pb.h>
#include <server/Player.h>
#include <server/Server.h>

namespace {

std::mt19937_64 &getGenerator() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    return gen;
}

std::size_t getRandomInt(std::size_t max) {
    std::uniform_int_distribution<std::size_t> distribution(0U, max);
    return distribution(getGenerator());
}

bool getRandomBool() {
    std::uniform_int_distribution<std::size_t> distribution(0U, 1U);
    return bool(distribution(getGenerator()));
}

float getRandomUniformFloat() {
    std::uniform_real_distribution<float> distribution(0.0F, 1.0F);
    return distribution(getGenerator());
}

} // namespace

IPlayer *ServerLogic::addPlayer(std::string const &userName, std::string const &displayName) {
    std::shared_ptr<IPlayer> player = std::make_shared<Player>(
        Player::Params{.username = userName, .displayName = displayName});

    std::lock_guard<std::mutex> const lock(m_playersMutex);
    // Insertion is not thread safe, so we have to ensure only one thread inserts a new player
    // at a time.
    auto [iter, success] = m_players.emplace(player.get(), player);
    if (!success) {
        return nullptr;
    }

    return iter->first;
}

IPlayer *ServerLogic::findPlayer(std::string const &userName, std::string const &displayName) {
    std::lock_guard<std::mutex> const lock(m_playersMutex);
    auto iter = std::find_if(m_players.begin(), m_players.end(), [&](auto const &pair) {
        return pair.first->getUsername() == userName &&
               pair.first->getDisplayName() == displayName;
    });
    if (iter == m_players.end()) {
        return nullptr;
    }
    return iter->first;
}

bool ServerLogic::addActivePlayer(IPlayer *player) {
    std::lock_guard<std::mutex> const lock(m_activePlayersMutex);
    auto [iter, success] = m_activePlayers.emplace(player->getConnection(), player);
    return success;
}

bool ServerLogic::removeActivePlayer(IPlayer *player) {
    std::lock_guard<std::mutex> const lock(m_activePlayersMutex);
    return bool(m_activePlayers.erase(player->getConnection()));
}

IPlayer *ServerLogic::getActivePlayer(ConnectionPtr hdl) {
    std::lock_guard<std::mutex> const lock(m_activePlayersMutex);
    auto iter = m_activePlayers.find(hdl);
    if (iter == m_activePlayers.end()) {
        return nullptr;
    }
    return iter->second;
}

GameInstance *ServerLogic::createGameInstance(IPlayer *player1, IPlayer *player2) {
    auto game =
        std::make_shared<GameInstance>(GameInstance{.player1 = player1, .player2 = player2});

    auto mappedItem = std::make_pair(game.get(), std::move(game));

    std::lock_guard<std::mutex> const lock(m_gamesMutex);
    auto [iter1, success1] = activeGames[player1->getConnection()].insert(mappedItem);
    auto [iter2, success2] = activeGames[player2->getConnection()].insert(mappedItem);

    return mappedItem.first;
}

bool ServerLogic::removeGameInstance(GameInstance *game) {
    auto hdl1 = game->player1->getConnection();
    auto hdl2 = game->player2->getConnection();

    std::lock_guard<std::mutex> const lock(m_gamesMutex);
    return bool(activeGames[hdl1].erase(game)) && bool(activeGames[hdl2].erase(game));
}

void ServerLogic::sendErrorResponse(ConnectionPtr hdl, std::string const &error) {
    game_proto::Response response;
    response.mutable_error()->set_msg(error.c_str());
    auto serializedResponse = response.SerializeAsString();

    auto sharedConnectionPtr = m_server->get_con_from_hdl(hdl);
    if (sharedConnectionPtr) {
        sharedConnectionPtr->send(serializedResponse.data(),
                                  serializedResponse.size(),
                                  websocketpp::frame::opcode::value::text);
    } else {
        std::cerr << "Could not send response after invalid request: client disconnected."
                  << std::endl;
    }
}

void ServerLogic::decodeAndProcessRequest(ConnectionPtr hdl, Server::message_ptr msg) {

    std::string payload = msg->get_payload();

    game_proto::Request request;
    if (!request.ParseFromString(payload)) {

        return sendErrorResponse(
            hdl, "Failed to parse request. Please ensure that the request is valid.");
    }

    if (request.has_registration_request()) {
        return processRegistrationRequest(hdl, request.registration_request());
    }
}

void ServerLogic::processRegistrationRequest(ConnectionPtr hdl,
                                             game_proto::RegistrationRequest const &request) {

    auto username = request.username();
    if (username.empty()) {
        return sendErrorResponse(hdl, "Received empty username. Username cannot be empty.");
    }
    auto displayName = request.display_name();
    if (displayName.empty()) {
        return sendErrorResponse(hdl,
                                 "Received empty display name. Display name cannot be empty.");
    }

    IPlayer *player = findPlayer(username, displayName);
    if (player != nullptr) {
        return sendErrorResponse(hdl, "Player already exists.");
    }

    player = addPlayer(username, displayName);
    if (player == nullptr) {
        return sendErrorResponse(hdl, "Could not add player.");
    }
    if (!addActivePlayer(player)) {
        return sendErrorResponse(hdl, "Could not add player.");
    }
}

void ServerLogic::processNewGameRequest(ConnectionPtr hdl,
                                        game_proto::NewGameRequest const &request) {

    auto player1 = getActivePlayer(hdl);
    std::uint32_t player1Rating = player1->getRating();

    std::size_t activePlayerCount = m_activePlayers.size();

    if (activePlayerCount < 2) {
        return sendErrorResponse(hdl, "Not enough players.");
    }

    IPlayer *player2;
    while (true) {
        auto player2Iter = m_activePlayers.begin();

        std::size_t playerIdx = getRandomInt(activePlayerCount);
        std::advance(player2Iter, playerIdx);

        player2 = player2Iter->second;

        if (player2 == player1) {
            continue;
        }

        std::uint32_t player2Rating = player2->getRating();
        std::uint32_t ratingDiff = player1Rating > player2Rating
                                       ? player1Rating - player2Rating
                                       : player2Rating - player1Rating;
        // We accept the player with the probability 1 / ( 1 + rating_diff^2).

        if (getRandomUniformFloat() < (1.0F / (1.0F + ratingDiff * ratingDiff))) {
            break;
        }
    }

    // Choose first move player. First player always starts.
    GameInstance *gameInstance = nullptr;
    if (getRandomBool()) {
        gameInstance = createGameInstance(player1, player2);
    } else {
        gameInstance = createGameInstance(player2, player1);
    }
}

void ServerLogic::sendNewGameResponse(GameInstance *game) {

    game_proto::Response response;
    auto &newGameResponse = *response.mutable_new_game_response();

    // Game id is just pointer to the game reinterpreted as a string. This way we can easily
    // recover the game on the move message.
    newGameResponse.set_game_id(reinterpret_cast<char *>(game), sizeof(GameInstance *));
    newGameResponse.set_make_first_move(true);

    newGameResponse.set_opponent_display_name(game->player2->getDisplayName());
    newGameResponse.set_opponent_rating(game->player2->getRating());
    auto serializedResponse = response.SerializeAsString();
    auto sharedConnectionPtr = m_server->get_con_from_hdl(game->player1->getConnection());

    sharedConnectionPtr->send(serializedResponse.data(),
                              serializedResponse.size(),
                              websocketpp::frame::opcode::value::text);

    newGameResponse.set_opponent_display_name(game->player1->getDisplayName());
    newGameResponse.set_opponent_rating(game->player1->getRating());
    serializedResponse = response.SerializeAsString();
    sharedConnectionPtr = m_server->get_con_from_hdl(game->player1->getConnection());
    sharedConnectionPtr->send(serializedResponse.data(),
                              serializedResponse.size(),
                              websocketpp::frame::opcode::value::text);
}