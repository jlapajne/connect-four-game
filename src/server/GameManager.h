#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <map>
#include <memory>
#include <mutex>

#include <server/ConnectFourGame.h>
#include <server/ConnectionMetadata.h>
#include <server/ServerTypes.h>

class GameManager {

    using GameMap = std::map<GameHdl, GamePtr>;

  public:
    using GameId = std::string;

    GameHdl createGameInstance(PlayerHdl player1, PlayerHdl player2);
    bool removeGameInstance(GameHdl game);
    bool removePlayer(ConnectionHdl connection);

    GamePtr getGame(ConnectionHdl connection, GameHdl game);
    GameMap *getGames(ConnectionHdl connection);
    PlayerPtr getOpponent(GameHdl Game, PlayerHdl player);

  public:
    // These could just as well be standalone functions.
    static std::string getGameId(GameHdl game);
    static GameHdl getGameFromId(std::string id);

  private:
    std::mutex m_mutex;

    // Games currently in progress.
    std::map<ConnectionHdl, GameMap, std::owner_less<ConnectionHdl>> m_activeGames;
};

#endif