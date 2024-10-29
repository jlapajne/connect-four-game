#ifndef CONNECT_FOUR

#include <array>
#include <cassert>
#include <cstdint>
#include <format>
#include <span>
#include <stdexcept>

#include <server/Player.h>

enum class CoinValue : std::uint8_t {
    Empty = 0,
    Red = 1,
    Blue = 2

};

class ConnectFourGame {

  public:
    std::uint32_t getFlatIndex(std::uint32_t rowIdx, std::uint32_t columnIdx) const;

    // public getters and setters
    std::span<CoinValue> getColumn(std::uint32_t columnIdx);
    std::span<CoinValue const> getColumn(std::uint32_t columnIdx) const;

    CoinValue &operator()(std::uint32_t rowIdx, std::uint32_t columnIdx);
    CoinValue const &operator()(std::uint32_t rowIdx, std::uint32_t columnIdx) const;

    void insertCoin(std::uint32_t columnIdx, CoinValue coin);
    bool checkIfWin(std::uint32_t columnIdx) const;

    std::uint32_t getMoveCount() const { return moveCount; }

  private:
    bool checkIfFourInColumn(std::uint32_t columnIdx) const;
    bool checkIfFourInRow(std::uint32_t columnIdx) const;
    bool checkIfFourInDiagonal(std::uint32_t columnIdx) const;

  private:
    static constexpr std::uint32_t WinningCoinStreak = 4;
    static constexpr std::uint32_t ColumnCount = 7;
    static constexpr std::uint32_t RowCount = 6;
    static constexpr std::uint32_t FlatBoardSize = ColumnCount * RowCount;

    // 7 columns and 6 rows board. Initialize everything to zero.
    std::array<CoinValue, FlatBoardSize> board{};
    std::array<std::uint32_t, ColumnCount> columnOccupancy{};
    std::uint32_t moveCount;
};

struct GameInstance {
    // Player one always starts the game.
    IPlayer *player1;
    IPlayer *player2;

    ConnectFourGame game;
};
#endif