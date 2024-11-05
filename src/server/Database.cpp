#include <sqlite3.h>

#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

#include <server/Database.h>

namespace {
void checkSqlStatus(int status) {
    if (status != SQLITE_OK && status != SQLITE_DONE) {
        auto msg = std::format("Failed to execute sql statement. Error code: {:d}", status);
        std::cerr << msg;
        throw std::runtime_error(msg);
    }
}

void checkAndFreeSqlErrorMsg(char *errMsg) {
    if (errMsg) {
        std::cerr << errMsg;
        std::string errorString(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error(errorString);
    }
}
} // namespace

Database::Database(std::filesystem::path const &playerDatabasePath,
                   std::filesystem::path const &gamesDatabasePath) {
    sqlite3_open(":memory:", &m_db);

    char *errMsg = nullptr;
    std::string attachPlayers = std::format(
        "ATTACH DATABASE {:s} AS {:s};", playerDatabasePath.string(), PlayersDatabaseName);
    sqlite3_exec(m_db, attachPlayers.c_str(), nullptr, nullptr, &errMsg);
    checkAndFreeSqlErrorMsg(errMsg);

    std::string attachGames = std::format(
        "ATTACH DATABASE {:s} AS {:s};", gamesDatabasePath.string(), GamesDatabaseName);
    sqlite3_exec(m_db, attachPlayers.c_str(), nullptr, nullptr, &errMsg);
    checkAndFreeSqlErrorMsg(errMsg);

    sqlite3_exec(m_db, attachGames.c_str(), nullptr, nullptr, &errMsg);
    checkAndFreeSqlErrorMsg(errMsg);

    createGamesTable();
    createPlayersTable();
}

Database::~Database() {
    if (m_db) {
        sqlite3_close(m_db);
    }
}

void Database::createPlayersTable() {

    static std::string createStatement =
        std::format("CREATE TABLE IF NOT EXISTS {:s}.players ("
                    "  username TEXT NOT NULL, "
                    "  display_name TEXT NOT NULL, "
                    "  rating INTEGER DEFAULT 1500, "
                    "  wins INTEGER DEFAULT 0, "
                    "  losses INTEGER DEFAULT 0, "
                    "  draws INTEGER DEFAULT 0, "
                    "  PRIMARY KEY(username, display_name));",
                    PlayersDatabaseName);
    sqlite3_exec(m_db, createStatement.c_str(), nullptr, nullptr, nullptr);
}

sqlite3_stmt *Database::prepareSimpleStatement(std::string const &statement) {
    sqlite3_stmt *ppStmt;
    int status =
        sqlite3_prepare_v2(m_db,                      /* Database handle */
                           /*zSql=*/statement.data(), /* SQL statement, UTF-8 encoded */
                           statement.size(),          /* Maximum length of zSql in bytes. */
                           &ppStmt,                   /* OUT: Statement handle */
                           nullptr /* OUT: Pointer to unused portion of zSql */
        );
    if (status != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare sql statement.");
    }
    return ppStmt;
}

void Database::insertPlayer(IPlayer const *player) {

    auto username = player->getUsername();
    auto displayName = player->getDisplayName();

    // Multi-thread. In this mode, SQLite can be safely used by multiple threads provided that
    // no single database connection nor any object derived from database connection, such as a
    // prepared statement, is used in two or more threads at the same time.
    // We prepare thread local objects here.
    thread_local std::string insertStatement = std::format(
        "INSERT INTO {:s}.players VALUES (?1, ?2, 1500, 0, 0, 0);", PlayersDatabaseName);
    thread_local sqlite3_stmt *preparedStatement = prepareSimpleStatement(insertStatement);
    checkSqlStatus(sqlite3_reset(preparedStatement));

    checkSqlStatus(
        sqlite3_bind_text(preparedStatement, 1, username.data(), username.size(), nullptr));
    checkSqlStatus(sqlite3_bind_text(
        preparedStatement, 2, displayName.data(), displayName.size(), nullptr));

    checkSqlStatus(sqlite3_step(preparedStatement));
}

void Database::createGamesTable() {

    static std::string createStatement =
        std::format("CREATE TABLE IF NOT EXISTS {:s}.games ("
                    "  player INTEGER FOREIGN KEY REFERENCES {:s}(rowid), "
                    "  opponent INTEGER FOREIGN KEY REFERENCES {:s}(rowid), "
                    "  winner INTEGER FOREIGN KEY REFERENCES {:s}(rowid), ",
                    GamesDatabaseName,
                    PlayersDatabaseName,
                    PlayersDatabaseName,
                    PlayersDatabaseName);

    char *errMsg = nullptr;
    checkSqlStatus(sqlite3_exec(m_db, createStatement.c_str(), nullptr, nullptr, &errMsg));
    checkAndFreeSqlErrorMsg(errMsg);
}

void Database::insertGame(GameInstance const *game) {

    // TODO: Add proper database querry for rowids of player and opponent.
    std::int64_t playerRowId = 1;
    std::int64_t opponentRowId = 1;
    std::int64_t winnerRowId = 1;

    thread_local std::string insertStatement =
        std::format("INSERT INTO {:s}.games VALUES (?1, ?2, ?3);", GamesDatabaseName);
    thread_local sqlite3_stmt *preparedStatement = prepareSimpleStatement(insertStatement);
    checkSqlStatus(sqlite3_reset(preparedStatement));

    checkSqlStatus(sqlite3_bind_int(preparedStatement, 1, playerRowId));
    checkSqlStatus(sqlite3_bind_int(preparedStatement, 2, opponentRowId));
    checkSqlStatus(sqlite3_bind_int(preparedStatement, 3, winnerRowId));

    checkSqlStatus(sqlite3_step(preparedStatement));
}
