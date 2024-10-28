#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <game.pb.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <server/ConnectFourGame.h>
#include <server/Player.h>
#include <set>
#include <string>

class ServerLogic {
  public:
    using ConnectionPtr = websocketpp::connection_hdl;

    Player const *addPlayer(std::string const &userName, std::string const &displayName) {
        auto player =
            std::make_shared<Player>(Player{.username = userName, .displayName = displayName});
        auto [iter, success] = players.insert({player.get(), std::move(player)});
        return iter->first;
    }

    bool removePlayer(Player *player) { return bool(players.erase(player)); }

    std::optional<std::shared_ptr<Player>> findPlayer(std::string const &userName,
                                                      std::string const &displayName) {
        auto iter = std::find_if(players.begin(), players.end(), [&](auto const &pair) {
            return pair.first->username == userName && pair.first->displayName == displayName;
        });
        if (iter == players.end()) {
            return std::nullopt;
        }
        return iter->second;
    }

    GameInstance const *createGameInstance(Player *player1, Player *player2) {
        auto game = std::make_shared<GameInstance>(
            GameInstance{.player1 = player1, .player2 = player2});

        auto mappedItem = std::make_pair(game.get(), std::move(game));

        auto [iter1, success1] = activeGames[player1->hdl].insert(mappedItem);
        auto [iter2, success2] = activeGames[player2->hdl].insert(mappedItem);

        return mappedItem.first;
    }

    bool removeGameInstance(GameInstance *game) {
        auto hdl1 = game->player1->hdl;
        auto hdl2 = game->player2->hdl;
        return bool(activeGames[hdl1].erase(game)) && bool(activeGames[hdl1].erase(game));
    }

  private:
    std::map<Player *, std::shared_ptr<Player>> players;

    using GameMap = std::map<GameInstance *, std::shared_ptr<GameInstance>>;
    std::map<ConnectionPtr, GameMap, std::owner_less<ConnectionPtr>> activeGames;
};

#endif