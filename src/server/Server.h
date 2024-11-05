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

class ServerLogic;
class Server : virtual public ServerType, public std::enable_shared_from_this<Server> {

  public:
    struct Params {
        std::uint32_t port = 9000;
        std::size_t maxTaskThreads = 10;
    };

    Server(Params params);

    ConnectionMetadata::Status getConnectionStatus(ConnectionHdl hdl) const;

  private:
    void onMessage(ConnectionHdl hdl, MessagePtr msg);
    void onConnectionClosed(ConnectionHdl hdl);
    void onConnectionOpened(ConnectionHdl hdl);
    ConnectionPtr getConnectionPtr(ConnectionHdl hdl);

  private:
    ConnectionList m_connectionList;
    asio::thread_pool m_threadPool;

    // pimpl -like implementation of logic.
    friend class SeverLogic;
    std::unique_ptr<ServerLogic> m_logic;
};

class ServerLogic {
  public:
    ServerLogic(Server *parentPtr) : m_server(parentPtr) {}

    void decodeAndProcessRequest(ConnectionHdl hdl, MessagePtr msg);

    void onConnectionClosed(ConnectionHdl hdl);

  private:
    void processProtoRequest(ConnectionHdl hdl, game_proto::Request const &request);
    void sendProtoMessage(ConnectionHdl hdl, google::protobuf::Message const &message);

    void sendErrorResponse(ConnectionHdl hdl,
                           std::string const &error,
                           std::optional<game_proto::ErrorCode> errorCode = std::nullopt);

    std::optional<std::string>
    validateUserCredentials(game_proto::UserCredentials const &credentials);

    void sendSuccessResponse(ConnectionHdl hdl);

    void
    sendGameEndResponse(IPlayer *p, std::string const &gameId, game_proto::GameEnd endResult);

    void processRegistrationRequest(ConnectionHdl hdl,
                                    game_proto::RegistrationRequest const &request);

    void processNewGameRequest(ConnectionHdl hdl, game_proto::NewGameRequest const &request);
    void processMoveRequest(ConnectionHdl hdl, game_proto::MoveRequest const &request);
    void processMessageRequest(ConnectionHdl hdl, game_proto::MessageRequest const &request);

    PlayerPtr getOpponent(GameHdl Game, PlayerHdl player);

  private:
    PlayerManager m_playerManager;
    GameManager m_gameManager;
    Server *m_server;
};

#endif