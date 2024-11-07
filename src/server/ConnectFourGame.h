#ifndef CONNECT_FOUR_GAME_H
#define CONNECT_FOUR_GAME_H

#include <array>
#include <cassert>
#include <cstdint>
#include <format>
#include <mutex>
#include <span>
#include <stdexcept>

#include <server/Player.h>

enum class CoinValue : std::uint8_t {
    Empty = 0,
    Player = 1,
    Opponent = 2

};

class ConnectFourGame {
  public:
    enum class Status : std::uint8_t { NotStarted = 0, InProgress = 1, Finished = 2 };

    // 7 columns and 6 rows board.
    static constexpr std::uint32_t WinningCoinStreak = 4;
    static constexpr std::uint32_t ColumnCount = 7;
    static constexpr std::uint32_t RowCount = 6;
    static constexpr std::uint32_t FlatBoardSize = ColumnCount * RowCount;

  public:
    // Public methods

    std::uint32_t getFlatIndex(std::uint32_t rowIdx, std::uint32_t columnIdx) const;

    // public getters and setters
    std::span<CoinValue const> getColumn(std::uint32_t columnIdx) const;

    CoinValue const &operator()(std::uint32_t rowIdx, std::uint32_t columnIdx) const;

    void insertPlayerCoin(std::uint32_t columnIdx);
    void insertOpponentCoin(std::uint32_t columnIdx);

    bool checkIfWin(std::uint32_t columnIdx) const;

    std::vector<std::uint32_t> getAvailableColumns() const;

    std::uint32_t getMoveCount() const { return m_moveCount; }
    bool isFull() const { return m_moveCount == FlatBoardSize; }

    void setStatus(Status status) { m_status = status; }
    Status getStatus() const { return m_status; }

  private:
    // Private methods
    void insertCoin(std::uint32_t columnIdx, CoinValue coin);
    bool checkIfFourInColumn(std::uint32_t columnIdx) const;
    bool checkIfFourInRow(std::uint32_t columnIdx) const;
    bool checkIfFourInDiagonal(std::uint32_t columnIdx) const;

  private:
    // Private variables.
    //  Initialize everything to zero.
    std::array<CoinValue, FlatBoardSize> m_board{};
    std::array<std::uint32_t, ColumnCount> m_columnOccupancy{};
    Status m_status = Status::NotStarted;
    std::uint32_t m_moveCount = 0U;
};

struct GameInstance {
    // Player one always starts the game.
    IPlayer *player1;
    IPlayer *player2;

    ConnectFourGame game;
};

using GameHdl = GameInstance *;
using GamePtr = std::shared_ptr<GameInstance>;

#endif