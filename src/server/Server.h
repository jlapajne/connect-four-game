#ifndef SERVER_H
#define SERVER_H

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

    void decodeAndProcessRequest(ConnectionPtr hdl, Server::message_ptr msg);

  private:
    IPlayer *addPlayer(std::string const &userName, std::string const &displayName);
    // clang-format off
    IPlayer *findPlayer(
        std::string const &userName, std::string const &displayName);
    // clang-format on

    bool addActivePlayer(IPlayer *player);
    bool removeActivePlayer(IPlayer *player);
    IPlayer *getActivePlayer(ConnectionPtr hdl);

    GameInstance *createGameInstance(IPlayer *player1, IPlayer *player2);
    bool removeGameInstance(GameInstance *game);

    void sendErrorResponse(ConnectionPtr hdl, std::string const &error);
    void processRegistrationRequest(ConnectionPtr hdl,
                                    game_proto::RegistrationRequest const &request);
    void processNewGameRequest(ConnectionPtr hdl, game_proto::NewGameRequest const &request);
    void sendNewGameResponse(GameInstance *game);

  private:
    std::mutex m_playersMutex;
    // Player are never removed from this map. Once registered, it's here forever.
    std::map<IPlayer *, std::shared_ptr<IPlayer>> m_players;

    std::mutex m_activePlayersMutex;
    // Players with active connection. IPlayer* pointer will always be valid, because this
    // map is only a subset of the above.
    std::map<ConnectionPtr, IPlayer *, std::owner_less<ConnectionPtr>> m_activePlayers;

    std::mutex m_gamesMutex;
    using GameMap = std::map<GameInstance *, std::shared_ptr<GameInstance>>;
    // Games currently in progress.
    std::map<ConnectionPtr, GameMap, std::owner_less<ConnectionPtr>> activeGames;

    std::shared_ptr<Server> m_server;
};

#endif