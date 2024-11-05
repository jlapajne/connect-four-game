#include "PlayerManager.h"

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <server/ConnectionMetadata.h>
#include <server/Player.h>
#include <server/RandomUtils.h>

PlayerPtr PlayerManager::addPlayer(std::string const &userName,
                                   std::string const &displayName,
                                   ConnectionHdl hdl) {
    std::shared_ptr<IPlayer> player = std::make_shared<Player>(Player::Params{
        .username = userName,
        .displayName = displayName,
        .hdl = std::move(hdl),
    });

    std::lock_guard<std::recursive_mutex> lock(m_playersMutex);
    // Insertion is not thread safe, so we have to ensure only one thread inserts a new player
    // at a time.
    auto [iter, success] = m_players.emplace(player.get(), player);
    if (!success) {
        return nullptr;
    }

    return iter->second;
}

PlayerPtr PlayerManager::findPlayer(std::string const &userName,
                                    std::string const &displayName) {
    std::lock_guard<std::recursive_mutex> lock(m_playersMutex);
    auto iter = std::find_if(m_players.begin(), m_players.end(), [&](auto const &pair) {
        return pair.first->getUsername() == userName &&
               pair.first->getDisplayName() == displayName;
    });
    if (iter == m_players.end()) {
        return nullptr;
    }
    return iter->second;
}

PlayerPtr PlayerManager::getPlayer(PlayerHdl player) {

    std::lock_guard<std::recursive_mutex> lock(m_playersMutex);
    auto playerIter = m_players.find(player);

    if (playerIter == m_players.end()) {
        return nullptr;
    }
    return playerIter->second;
}

bool PlayerManager::addActivePlayer(PlayerHdl player) {
    std::lock_guard<std::recursive_mutex> lock(m_activePlayersMutex);
    auto hdl = player->getConnection();
    auto [iter, success] = m_activePlayers.insert(std::make_pair(hdl, player));
    return success;
}

bool PlayerManager::removeActivePlayer(PlayerHdl player) {
    std::lock_guard<std::recursive_mutex> lock(m_activePlayersMutex);
    return bool(m_activePlayers.erase(player->getConnection()));
}

PlayerPtr PlayerManager::getActivePlayer(ConnectionHdl hdl) {
    std::lock_guard<std::recursive_mutex> lock(m_activePlayersMutex);
    auto iter = m_activePlayers.find(hdl);
    if (iter == m_activePlayers.end()) {
        return nullptr;
    }

    PlayerHdl playerHdl = iter->second;
    auto playerIter = m_players.find(playerHdl);
    if (playerIter == m_players.end()) {
        return nullptr;
    }

    return playerIter->second;
}

std::size_t PlayerManager::activePlayerCount() {
    std::lock_guard<std::recursive_mutex> lock(m_activePlayersMutex);
    return m_activePlayers.size();
}

PlayerPtr PlayerManager::selectOpponentForPlayer(PlayerHdl player) {

    std::uint32_t playerRating = player->getRating();

    PlayerHdl opponent;
    while (true) {
        std::lock_guard<std::recursive_mutex> lock(m_activePlayersMutex);

        std::size_t nActivePlayers = activePlayerCount();
        auto opponentIter = m_activePlayers.begin();

        // Randomly select the opponent.
        std::size_t opponentIdx = getRandomInt(nActivePlayers);
        std::advance(opponentIter, opponentIdx);

        opponent = opponentIter->second;

        if (opponent == player) {
            continue;
        }

        std::size_t opponentRating = opponent->getRating();
        std::uint32_t ratingDiff = playerRating > opponentRating
                                       ? playerRating - opponentRating
                                       : opponentRating - playerRating;

        // We accept the player with the probability 1 / ( 1 + rating_diff^2).
        if (getRandomUniformFloat() < (1.0F / (1.0F + ratingDiff * ratingDiff))) {
            return m_players[opponent];
        }
    }
}
