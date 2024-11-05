#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <server/Player.h>
#include <server/ServerTypes.h>

class PlayerManager {
  public:
    PlayerHdl addPlayer(std::string const &userName, std::string const &displayName);

    // clang-format off
    PlayerPtr findPlayer(
        std::string const &userName, std::string const &displayName);
    // clang-format on

    PlayerPtr getPlayer(PlayerHdl player);

    bool addActivePlayer(PlayerHdl player);
    bool removeActivePlayer(PlayerHdl player);
    PlayerPtr getActivePlayer(ConnectionHdl hdl);

    std::size_t activePlayerCount();

    PlayerPtr selectOpponentForPlayer(PlayerHdl player);

  private:
    std::recursive_mutex m_playersMutex;
    // Player are never removed from this map. Once registered, it's here forever.
    std::map<PlayerHdl, PlayerPtr> m_players;

    std::recursive_mutex m_activePlayersMutex;
    // Players with active connection. IPlayer* pointer will always be valid, because this
    // map is only a subset of the above.
    std::map<ConnectionHdl, PlayerHdl, std::owner_less<ConnectionHdl>> m_activePlayers;
};

#endif