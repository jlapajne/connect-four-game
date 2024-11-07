

#include "GameManager.h"

#include <server/ConnectionMetadata.h>
#include <server/ServerTypes.h>

#include <mutex>
#include <string_view>
#include <utility>

GameHdl GameManager::createGameInstance(IPlayer *player1, IPlayer *player2) {
    auto game =
        std::make_shared<GameInstance>(GameInstance{.player1 = player1, .player2 = player2});

    auto mappedItem = std::make_pair(game.get(), std::move(game));

    std::lock_guard<std::mutex> lock(m_mutex);
    auto [iter1, success1] = m_activeGames[player1->getConnection()].insert(mappedItem);
    auto [iter2, success2] = m_activeGames[player2->getConnection()].insert(mappedItem);

    return mappedItem.first;
}

bool GameManager::removeGameInstance(GameHdl game) {
    auto hdl1 = game->player1->getConnection();
    auto hdl2 = game->player2->getConnection();

    std::lock_guard<std::mutex> lock(m_mutex);
    return bool(m_activeGames[hdl1].erase(game)) && bool(m_activeGames[hdl2].erase(game));
}

auto GameManager::getGameId(GameHdl game) -> GameId { return std::size_t(game); }

GameHdl GameManager::getGameFromId(GameId id) { return reinterpret_cast<GameHdl>(id); }

GamePtr GameManager::getGame(ConnectionHdl conHdl, GameHdl gameHdl) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto games = m_activeGames.find(conHdl);

    if (games != m_activeGames.end()) {
        auto gameIter = games->second.find(gameHdl);
        if (gameIter != games->second.end()) {
            return gameIter->second;
        }
    }
    return nullptr;
}

GameManager::GameMap *GameManager::getGames(ConnectionHdl connection) {
    if (m_activeGames.contains(connection)) {
        return &m_activeGames[connection];
    }
    return nullptr;
}

bool GameManager::removePlayer(ConnectionHdl connection) {
    return bool(m_activeGames.erase(connection));
}
