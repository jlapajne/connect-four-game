#ifndef SERVER_H
#define SERVER_H

#include <functional>
#include <iostream>
#include <memory>

#include <asio/post.hpp>
#include <asio/thread_pool.hpp>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <server/ConnectFourGame.h>
#include <server/Player.h>
#include <server/ServerTypes.h>

class ServerLogic;
class Server : public BaseServerType, public std::enable_shared_from_this<Server> {

  public:
    struct Params {
        std::uint32_t port = 9000;
        std::size_t maxTaskThreads = 1;
    };

    Server(Params params);

  private:
    void onMessage(ConnectionPtr hdl, MessagePtr msg);

  private:
    std::unique_ptr<ServerLogic> m_logic;
    asio::thread_pool m_threadPool;
};

class ServerLogic {
  public:
    ServerLogic(std::shared_ptr<Server> parentPtr) : m_server(std::move(parentPtr)) {}

    IPlayer const *addPlayer(std::string const &userName, std::string const &displayName);
    bool removePlayer(IPlayer *player);

    // clang-format off
    std::optional<std::shared_ptr<IPlayer>> findPlayer(
        std::string const &userName, std::string const &displayName);
    // clang-format on

    GameInstance const *createGameInstance(IPlayer *player1, IPlayer *player2);
    bool removeGameInstance(GameInstance *game);

    void decodeAndProcessRequest(ConnectionPtr hdl, Server::message_ptr msg);

  private:
    void sendErrorResponse(std::string const &error, ConnectionPtr hdl);

  private:
    std::map<IPlayer *, std::shared_ptr<IPlayer>> m_players;

    using GameMap = std::map<GameInstance *, std::shared_ptr<GameInstance>>;
    std::map<ConnectionPtr, GameMap, std::owner_less<ConnectionPtr>> activeGames;

    std::shared_ptr<Server> m_server;
};

#endif