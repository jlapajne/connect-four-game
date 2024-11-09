#ifndef SERVER_H
#define SERVER_H

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

#include <asio/post.hpp>
#include <asio/thread_pool.hpp>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <game.pb.h>
#include <server/ConnectFourGame.h>
#include <server/ConnectionMetadata.h>
#include <server/GameManager.h>
#include <server/Player.h>
#include <server/PlayerManager.h>
#include <server/ServerTypes.h>

#pragma optimize("", off)

class ServerLogic;
class Server : virtual public ServerType, public std::enable_shared_from_this<Server> {

  public:
    struct Params {
        std::uint32_t port = 9000;
        std::size_t maxTaskThreads = 10;
    };

    Server(Params params);

    ConnectionMetadata::Status getConnectionStatus(ConnectionId id) const;

    void sendMessage(ConnectionId id, auto &&...args) {
        ConnectionHdl hdl = m_connections.at(id).getHdl();
        // Send is a base class ServerType method.
        this->send(hdl, std::forward<decltype(args)>(args)...);
    }

  private:
    void onMessage(ConnectionHdl hdl, MessagePtr msg);
    void onConnectionClosed(ConnectionHdl hdl);
    void onConnectionOpened(ConnectionHdl hdl);
    ConnectionPtr getConnectionPtr(ConnectionHdl hdl);

  private:
    asio::thread_pool m_threadPool;

    // Unordered map can not have weak_ptr as a key, so we need additional mapping.
    std::unordered_map<ConnectionId, ConnectionMetadata> m_connections;

    // pimpl-like implementation of logic.
    friend class SeverLogic;
    std::unique_ptr<ServerLogic> m_logic;
};

class ServerLogic {
  public:
    using GameId = GameManager::GameId;

    ServerLogic(Server *parentPtr) : m_server(parentPtr) {}

    void decodeAndProcessRequest(ConnectionId id, MessagePtr msg);

    void onConnectionClosed(ConnectionId id);

  private:
    void processProtoRequest(ConnectionId id, game_proto::Request const &request);
    void sendProtoMessage(ConnectionId id, google::protobuf::Message const &message);

    void sendErrorResponse(ConnectionId id,
                           std::string const &error,
                           std::optional<game_proto::ErrorCode> errorCode = std::nullopt);

    std::optional<std::string>
    validateUserCredentials(game_proto::UserCredentials const &credentials);

    void sendSuccessResponse(ConnectionId id);

    void sendGameEndResponse(PlayerHdl p, GameId gameId, game_proto::GameEnd result);

    void processRegistrationRequest(ConnectionId id,
                                    game_proto::RegistrationRequest const &request);

    void processNewGameRequest(ConnectionId id, game_proto::NewGameRequest const &request);
    void processMoveRequest(ConnectionId id, game_proto::MoveRequest const &request);
    void processMessageRequest(ConnectionId id, game_proto::MessageRequest const &request);

    PlayerPtr getOpponent(GameHdl Game, PlayerHdl player);

  private:
    PlayerManager m_playerManager;
    GameManager m_gameManager;
    Server *m_server;
};

#endif