

#include <server/ConnectFourGame.h>

#include <cstdint>

std::uint32_t ConnectFourGame::getFlatIndex(std::uint32_t rowIdx,
                                            std::uint32_t columnIdx) const {
    return rowIdx + columnIdx * RowCount;
}
// public getters and setters
std::span<CoinValue> ConnectFourGame::getColumn(std::uint32_t columnIdx) {
    assert(columnIdx < ColumnCount);
    return std::span<CoinValue>(board).subspan(columnIdx * RowCount, RowCount);
}

std::span<CoinValue const> ConnectFourGame::getColumn(std::uint32_t columnIdx) const {
    assert(columnIdx < ColumnCount);
    return std::span<CoinValue const>(board).subspan(columnIdx * RowCount, RowCount);
}

CoinValue &ConnectFourGame::operator()(std::uint32_t rowIdx, std::uint32_t columnIdx) {
    assert(columnIdx < ColumnCount);
    assert(rowIdx < RowCount);
    return board[getFlatIndex(rowIdx, columnIdx)];
}

CoinValue const &ConnectFourGame::operator()(std::uint32_t rowIdx,
                                             std::uint32_t columnIdx) const {
    assert(columnIdx < ColumnCount);
    assert(rowIdx < RowCount);
    return board[getFlatIndex(rowIdx, columnIdx)];
}

void ConnectFourGame::insertCoin(std::uint32_t columnIdx, CoinValue coin) {
    if (columnIdx >= ColumnCount) {
        throw std::invalid_argument(
            std::format("column index should be in range [0, {:d}]. Got {:d}.",
                        ColumnCount - 1,
                        columnIdx));
    }

    std::uint32_t rowIdx = columnOccupancy[columnIdx];
    if (rowIdx >= RowCount) {
        throw std::runtime_error("Column is already full.");
    }

    columnOccupancy[columnIdx]++;
    board[getFlatIndex(rowIdx, columnIdx)] = coin;
    moveCount++;
}

bool ConnectFourGame::checkIfFourInColumn(std::uint32_t columnIdx) const {
    assert(columnIdx < ColumnCount);

    std::uint32_t rowIdx = columnOccupancy[columnIdx];
    assert(rowIdx < RowCount);

    if (rowIdx < WinningCoinStreak - 1) {
        return false;
    }

    auto refCoin = board[getFlatIndex(rowIdx, columnIdx)];
    assert(refCoin != CoinValue::Empty);

    for (std::uint32_t i = 1; i < WinningCoinStreak; ++i) {
        std::uint32_t flatIdx = getFlatIndex(rowIdx - i, columnIdx);

        if (board[flatIdx] != refCoin) {
            return false;
        }
    }
    return true;
}

bool ConnectFourGame::checkIfFourInRow(std::uint32_t columnIdx) const {
    assert(columnIdx < ColumnCount);

    std::uint32_t rowIdx = columnOccupancy[columnIdx];
    assert(rowIdx < RowCount);

    auto refCoin = board[getFlatIndex(rowIdx, columnIdx)];
    assert(refCoin != CoinValue::Empty);

    std::uint32_t countInRow = 1;
    // Check right neigbhbors
    for (std::uint32_t c = columnIdx + 1; c < ColumnCount; ++c) {

        std::uint32_t flatIdx = getFlatIndex(rowIdx, c);

        if (board[flatIdx] != refCoin) {
            break;
        }
        countInRow++;
    }

    // Check left neighbours
    for (std::uint32_t c = columnIdx - 1;; --c) {
        std::uint32_t flatIdx = getFlatIndex(rowIdx, c);

        if (board[flatIdx] != refCoin) {
            break;
        }
        countInRow++;

        if (c == 0) {
            break;
        }
    }
    return countInRow >= WinningCoinStreak;
}

bool ConnectFourGame::checkIfFourInDiagonal(std::uint32_t columnIdx) const {

    assert(columnIdx < ColumnCount);

    std::uint32_t rowIdx = columnOccupancy[columnIdx];
    assert(rowIdx < RowCount);

    auto refCoin = board[getFlatIndex(rowIdx, columnIdx)];
    assert(refCoin != CoinValue::Empty);

    auto checkDiagonal = [&](std::int32_t coeffRow,
                             std::int32_t coeffColumn) -> std::uint32_t {
        std::uint32_t countInDiagonal = 0;
        for (std::uint32_t i = 1; i < WinningCoinStreak; ++i) {
            std::uint32_t flatIdx =
                getFlatIndex(rowIdx + i * coeffRow, columnIdx + i * coeffColumn);
            if (board[flatIdx] != refCoin) {
                break;
            }
            countInDiagonal++;
        }
        return countInDiagonal;
    };

    // Check y=x diagonal.
    std::uint32_t countInDiagonal1 = 1 + checkDiagonal(1, 1) + checkDiagonal(-1, -1);
    // Check y=-x diagonal.
    std::uint32_t countInDiagonal2 = 1 + checkDiagonal(1, -1) + checkDiagonal(-1, 1);

    return countInDiagonal1 >= WinningCoinStreak || countInDiagonal2 >= WinningCoinStreak;
}

bool ConnectFourGame::checkIfWin(std::uint32_t columnIdx) const {
    return checkIfFourInColumn(columnIdx) || checkIfFourInRow(columnIdx) ||
           checkIfFourInDiagonal(columnIdx);
}