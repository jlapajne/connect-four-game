

#include <server/ConnectFourGame.h>

#include <cstdint>

std::uint32_t ConnectFourGame::getFlatIndex(std::uint32_t rowIdx,
                                            std::uint32_t columnIdx) const {
    return rowIdx + columnIdx * RowCount;
}

std::span<CoinValue const> ConnectFourGame::getColumn(std::uint32_t columnIdx) const {
    assert(columnIdx < ColumnCount);
    return std::span<CoinValue const>(m_board).subspan(columnIdx * RowCount, RowCount);
}

CoinValue const &ConnectFourGame::operator()(std::uint32_t rowIdx,
                                             std::uint32_t columnIdx) const {
    assert(columnIdx < ColumnCount);
    assert(rowIdx < RowCount);
    return m_board[getFlatIndex(rowIdx, columnIdx)];
}

void ConnectFourGame::insertCoin(std::uint32_t columnIdx, CoinValue coin) {
    if (columnIdx >= ColumnCount) {
        throw std::invalid_argument(
            std::format("column index should be in range [0, {:d}]. Got {:d}.",
                        ColumnCount - 1,
                        columnIdx));
    }

    std::uint32_t rowIdx = m_columnOccupancy[columnIdx];
    if (rowIdx >= RowCount) {
        throw std::runtime_error("Column is already full.");
    }

    m_columnOccupancy[columnIdx]++;
    m_board[getFlatIndex(rowIdx, columnIdx)] = coin;
    m_moveCount++;
}

void ConnectFourGame::insertPlayer1Coin(std::uint32_t columnIdx) {
    return insertCoin(columnIdx, CoinValue::Player1);
}

void ConnectFourGame::insertPlayer2Coin(std::uint32_t columnIdx) {
    return insertCoin(columnIdx, CoinValue::Player2);
}

bool ConnectFourGame::checkIfFourInColumn(std::uint32_t columnIdx) const {
    assert(columnIdx < ColumnCount);

    std::uint32_t rowIdx = m_columnOccupancy[columnIdx];
    assert(rowIdx < RowCount);

    if (rowIdx < WinningCoinStreak - 1) {
        return false;
    }

    auto refCoin = m_board[getFlatIndex(rowIdx, columnIdx)];
    assert(refCoin != CoinValue::Empty);

    for (std::uint32_t i = 1; i < WinningCoinStreak; ++i) {
        std::uint32_t flatIdx = getFlatIndex(rowIdx - i, columnIdx);

        if (m_board[flatIdx] != refCoin) {
            return false;
        }
    }
    return true;
}

bool ConnectFourGame::checkIfFourInRow(std::uint32_t columnIdx) const {
    assert(columnIdx < ColumnCount);

    std::uint32_t rowIdx = m_columnOccupancy[columnIdx];
    // This function is only called after a coin has been put into a column, so occupancy is
    // expected to be greater than 0.
    assert(rowIdx > 0);
    rowIdx -= 1;

    assert(rowIdx < RowCount);

    auto refCoin = m_board[getFlatIndex(rowIdx, columnIdx)];
    assert(refCoin != CoinValue::Empty);

    std::uint32_t countInRow = 1;
    // Check right neigbhbors
    for (std::uint32_t c = columnIdx + 1; c < ColumnCount; ++c) {

        std::uint32_t flatIdx = getFlatIndex(rowIdx, c);

        if (m_board[flatIdx] != refCoin) {
            break;
        }
        countInRow++;
    }

    // Check left neighbours
    for (std::uint32_t c = columnIdx - 1;; --c) {
        std::uint32_t flatIdx = getFlatIndex(rowIdx, c);

        if (m_board[flatIdx] != refCoin) {
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

    std::uint32_t rowIdx = m_columnOccupancy[columnIdx] - 1;
    assert(rowIdx < RowCount);

    auto refCoin = m_board[getFlatIndex(rowIdx, columnIdx)];
    assert(refCoin != CoinValue::Empty);

    auto checkDiagonal = [&](std::int32_t coeffRow,
                             std::int32_t coeffColumn) -> std::uint32_t {
        std::uint32_t countInDiagonal = 0;
        for (std::uint32_t i = 1; i < WinningCoinStreak; ++i) {
            std::uint32_t flatIdx =
                getFlatIndex(rowIdx + i * coeffRow, columnIdx + i * coeffColumn);
            if (m_board[flatIdx] != refCoin) {
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

std::vector<std::uint32_t> ConnectFourGame::getAvailableColumns() const {

    std::vector<std::uint32_t> availableColumns;
    for (std::uint32_t i = 0; i < ColumnCount; ++i) {
        if (m_columnOccupancy[i] < RowCount) {
            availableColumns.push_back(i);
        }
    }
    return availableColumns;
}