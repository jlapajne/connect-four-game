#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <server/Player.h>
#include <server/ServerTypes.h>

class PlayerManager {
  public:
    PlayerPtr
    addPlayer(std::string const &userName, std::string const &displayName, ConnectionId id);

    // clang-format off
    PlayerPtr findPlayer(
        std::string const &userName, std::string const &displayName);
    // clang-format on

    PlayerPtr getPlayer(PlayerHdl player);

    bool addActivePlayer(PlayerHdl player);
    bool removeActivePlayer(PlayerHdl player);
    PlayerPtr getActivePlayer(ConnectionId id);

    std::size_t activePlayerCount();

    PlayerPtr selectOpponentForPlayer(PlayerHdl player);

  private:
    std::recursive_mutex m_playersMutex;
    // Player are never removed from this map. Once registered, it's here forever.
    std::unordered_map<PlayerHdl, PlayerPtr, std::hash<PlayerHdl>> m_players;

    std::recursive_mutex m_activePlayersMutex;
    // Players with active connection. IPlayer* pointer will always be valid, because this
    // map is only a subset of the above.
    std::unordered_map<ConnectionId, PlayerHdl> m_activePlayers;
};

#endif