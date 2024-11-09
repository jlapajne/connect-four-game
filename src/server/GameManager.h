#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include <server/ConnectFourGame.h>
#include <server/ConnectionMetadata.h>
#include <server/ServerTypes.h>

class GameManager {

  public:
    using GameId = std::size_t;

    GameHdl createGameInstance(PlayerHdl player1, PlayerHdl player2);
    bool removeGameInstance(GameHdl game);
    bool removePlayer(ConnectionId connection);

    GamePtr getGame(ConnectionId connection, GameHdl game);
    PlayerPtr getOpponent(GameHdl Game, PlayerHdl player);

    using GameMap = std::map<GameHdl, GamePtr>;
    GameMap *getGames(ConnectionId connection);

  public:
    // These could just as well be standalone functions.
    static GameId getGameId(GameHdl game);
    static GameHdl getGameFromId(GameId id);

  private:
    std::mutex m_mutex;

    // Games currently in progress.
    std::unordered_map<ConnectionId, GameMap> m_activeGames;
};

#endif