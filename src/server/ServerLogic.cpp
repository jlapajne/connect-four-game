

#include <format>
#include <iterator>
#include <mutex>
#include <optional>
#include <random>
#include <stdexcept>

#include <websocketpp/frame.hpp>

#include <game.pb.h>
#include <server/Player.h>
#include <server/RandomUtils.h>
#include <server/Server.h>

#include <google/protobuf/message.h>

std::optional<std::string>
ServerLogic::validateUserCredentials(game_proto::UserCredentials const &credentials) {

    auto checkField = [](std::string const &field,
                         std::string const &fieldName) -> std::optional<std::string> {
        if (field.empty()) {
            return std::format("Received empty {0} field. {0} cannot be empty.", fieldName);
        }

        // Some random name limitation.
        if (field.size() > 50) {
            return std::format("{0} cannot be longer than 50 characters.", fieldName);
        }

        return std::nullopt;
    };

    std::optional<std::string> opErr = checkField(credentials.username(), "username");
    if (opErr) {
        return opErr;
    }

    opErr = checkField(credentials.display_name(), "display name");
    if (opErr) {
        return opErr;
    }

    return std::nullopt;
}

void ServerLogic::sendProtoMessage(ConnectionHdl hdl,
                                   google::protobuf::Message const &message) {

    std::string msgAsString = message.SerializeAsString();
    try {
        m_server->send(hdl, msgAsString, websocketpp::frame::opcode::value::text);

    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
}

void ServerLogic::sendErrorResponse(ConnectionHdl hdl,
                                    std::string const &error,
                                    std::optional<game_proto::ErrorCode> errorCode) {
    game_proto::Response errorResponse;
    errorResponse.mutable_error()->set_msg(error.c_str());
    if (errorCode.has_value()) {
        errorResponse.mutable_error()->set_error_code(*errorCode);
    }
    sendProtoMessage(hdl, errorResponse);
}

void ServerLogic::sendSuccessResponse(ConnectionHdl hdl) {

    game_proto::Response successResponse;
    sendProtoMessage(hdl, successResponse);
}

void ServerLogic::decodeAndProcessRequest(ConnectionHdl hdl, MessagePtr msg) {

    std::string payload = msg->get_payload();

    game_proto::Request request;
    if (!request.ParseFromString(payload)) {
        return sendErrorResponse(
            hdl, "Failed to parse request. Please ensure that the request is valid.");
    }

    try {
        processProtoRequest(hdl, request);
    } catch (GameException const &gameException) {
        sendErrorResponse(hdl, gameException.what());
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        sendErrorResponse(hdl,
                          std::format("Failed to process request. Error {:s}", e.what()),
                          game_proto::ErrorCode::InvalidRequest);
    }
}

void ServerLogic::processProtoRequest(ConnectionHdl hdl, game_proto::Request const &request) {

    if (request.has_registration_request()) {
        return processRegistrationRequest(hdl, request.registration_request());
    } else if (request.has_new_game_request()) {
        return processNewGameRequest(hdl, request.new_game_request());
    } else if (request.has_move_request()) {
        return processMoveRequest(hdl, request.move_request());
    }
}

std::optional<std::string>
validateUserCredentials(game_proto::UserCredentials const &credentials) {

    auto const &username = credentials.username();
    if (username.empty()) {
        return "Received empty username. Username cannot be empty.";
    }
    auto const &displayName = credentials.display_name();
    if (displayName.empty()) {
        return "Received empty display name. Display name cannot be empty.";
    }
    return std::nullopt;
}

void ServerLogic::processRegistrationRequest(ConnectionHdl hdl,
                                             game_proto::RegistrationRequest const &request) {

    auto const &credentials = request.user_credentials();
    if (auto error = validateUserCredentials(credentials)) {
        sendErrorResponse(hdl, *error);
    }

    auto const &username = credentials.username();
    auto const &displayName = credentials.display_name();
    PlayerPtr player = m_playerManager.findPlayer(username, displayName);
    if (player) {
        return sendErrorResponse(hdl, "Player already exists.");
    }
    player = m_playerManager.addPlayer(username, displayName, hdl);
    if (!player) {
        return sendErrorResponse(hdl, "Could not add player.");
    }

    if (!m_playerManager.addActivePlayer(player.get())) {
        return sendErrorResponse(hdl, "Could not add active player.");
    }
    std::cout << std::format("Registered new user with username {:s} and display name {:s}.\n",
                             username,
                             displayName);

    game_proto::Response response;
    response.mutable_registration_success_response();
    sendProtoMessage(hdl, response);
}

void ServerLogic::processNewGameRequest(ConnectionHdl hdl,
                                        game_proto::NewGameRequest const &request) {

    std::cout << "Received new game request.\n";

    PlayerPtr player = m_playerManager.getActivePlayer(hdl);

    std::size_t activePlayerCount = m_playerManager.activePlayerCount();

    if (activePlayerCount < 2) {
        return sendErrorResponse(hdl, "Not enough players.");
    }

    // select opponent
    auto opponent = m_playerManager.selectOpponentForPlayer(player.get());

    // Choose first move player. First player always starts.
    GameHdl gameInstance = nullptr;
    bool playerStarts = getRandomBool();
    if (playerStarts) {
        gameInstance = m_gameManager.createGameInstance(player.get(), opponent.get());
    } else {
        gameInstance = m_gameManager.createGameInstance(opponent.get(), player.get());
    }

    auto prepareResponse = [gameInstance](bool startGame, PlayerHdl opponent) {
        game_proto::Response response;
        auto &newGameResponse = *response.mutable_new_game_response();
        newGameResponse.set_opponent_display_name(opponent->getDisplayName());
        newGameResponse.set_make_first_move(startGame);
        newGameResponse.set_opponent_rating(opponent->getRating());
        newGameResponse.set_game_id(GameManager::getGameId(gameInstance));
        return response;
    };

    sendProtoMessage(player->getConnection(), prepareResponse(playerStarts, opponent.get()));
    sendProtoMessage(opponent->getConnection(), prepareResponse(!playerStarts, player.get()));
}

void ServerLogic::sendGameEndResponse(PlayerHdl p,
                                      std::string const &gameId,
                                      game_proto::GameEnd endResult) {
    // Reponse for the winner.
    game_proto::Response response;
    game_proto::GameEndResponse &end_response = *response.mutable_game_end_response();
    end_response.set_game_id(gameId);
    end_response.set_game_end(endResult);
    sendProtoMessage(p->getConnection(), response);
};

PlayerPtr ServerLogic::getOpponent(GameHdl game, PlayerHdl player) {
    assert(game->player1 == player || game->player2 == player);
    ConnectionHdl connection = player->getConnection();
    GamePtr gamePtr = m_gameManager.getGame(connection, game);
    if (!game) {
        return nullptr;
    }

    auto opponentHandle = game->player1 == player ? game->player2 : game->player1;

    return m_playerManager.getPlayer(opponentHandle);
}

void ServerLogic::processMoveRequest(ConnectionHdl hdl,
                                     game_proto::MoveRequest const &request) {

    GameHdl gameInstance = GameManager::getGameFromId(request.game_id());
    GamePtr gamePtr = m_gameManager.getGame(hdl, gameInstance);
    if (!gamePtr) {
        return sendErrorResponse(
            hdl, std::format("Game is not active. Game {:s}.", request.game_id()));
    }

    PlayerPtr player = m_playerManager.getActivePlayer(hdl);
    std::uint32_t columnIdx = request.column_idx();

    auto &game = gamePtr->game;
    PlayerPtr opponent = getOpponent(gameInstance, player.get());

    if (!opponent) {
        return sendErrorResponse(hdl, "Opponent not found.");
    }

    bool hasWon = game.checkIfWin(columnIdx);
    bool gameEnd = game.isFull() || hasWon;

    game_proto::Response response;
    if (gameEnd) {
        sendGameEndResponse(player.get(),
                            request.game_id(),
                            hasWon ? game_proto::GameEnd::Win : game_proto::GameEnd::Draw);
        sendGameEndResponse(opponent.get(),
                            request.game_id(),
                            hasWon ? game_proto::GameEnd::Loss : game_proto::GameEnd::Draw);

        m_gameManager.removeGameInstance(gameInstance);
    } else {
        // If the player, that made the move, has not won, then we send available moves to
        // the other player, so that he makes the next move.
        game_proto::AvailableMovesResponse &rsp = *response.mutable_available_games_response();

        rsp.set_game_id(request.game_id());
        *rsp.mutable_column_idx() = google::protobuf::RepeatedField<uint32_t>(
            game.getAvailableColumns().begin(), game.getAvailableColumns().end());

        sendProtoMessage(opponent->getConnection(), response);
    }
}

void ServerLogic::processMessageRequest(ConnectionHdl hdl,
                                        game_proto::MessageRequest const &request) {
    auto const &gameId = request.game_id();

    GameHdl gameInstance = GameManager::getGameFromId(gameId);

    GamePtr game = m_gameManager.getGame(hdl, gameInstance);
    if (!game) {
        return sendErrorResponse(
            hdl,
            std::format("Message request refused. Game is not active. Game {:s}.", gameId));
    }

    PlayerPtr sender = m_playerManager.getActivePlayer(hdl);
    PlayerHdl receiver =
        sender.get() == gameInstance->player1 ? gameInstance->player2 : gameInstance->player1;

    game_proto::Response response;
    response.mutable_message_response()->set_game_id(gameId);
    response.mutable_message_response()->set_sender_display_name(sender->getDisplayName());
    response.mutable_message_response()->set_message(request.message());
    sendProtoMessage(receiver->getConnection(), response);
}

void ServerLogic::onConnectionClosed(ConnectionHdl hdl) {
    auto games = m_gameManager.getGames(hdl);

    if (!games) {
        m_gameManager.removePlayer(hdl);
        return;
    }

    PlayerPtr player = m_playerManager.getActivePlayer(hdl);
    assert(player != nullptr);

    for (auto &item : *games) {

        auto *gameInstance = item.first;
        auto opponent = getOpponent(gameInstance, player.get());

        sendGameEndResponse(
            opponent.get(), GameManager::getGameId(gameInstance), game_proto::GameEnd::Win);
        m_gameManager.removeGameInstance(gameInstance);
    }
    m_gameManager.removePlayer(hdl);
}