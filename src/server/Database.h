#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H

#include <sqlite3.h>

#include <filesystem>
#include <format>

#include <server/ConnectFourGame.h>
#include <server/Player.h>

class Database {
  public:
    Database(std::filesystem::path const &playerDatabasePath,
             std::filesystem::path const &gamesDatabasePath);

    ~Database();

    void insertPlayer(IPlayer const *player);
    void insertGame(GameInstance const *game);

  private:
    constexpr static std::string_view PlayersDatabaseName = "players_db";
    constexpr static std::string_view GamesDatabaseName = "games_db";
    constexpr static std::string_view PlayersTable = "players";
    constexpr static std::string_view GamesTable = "games";

    void createPlayersTable();
    void createGamesTable();

    sqlite3_stmt *prepareSimpleStatement(std::string const &statement);

  private:
    sqlite3 *m_db = nullptr;
};

#endif