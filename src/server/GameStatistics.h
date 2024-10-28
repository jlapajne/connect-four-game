
#include <optional>

#include <server/Player.h>

struct GameStat {
    Player player1;
    Player player2;
    std::uint32_t moveCount = 0;
    std::optional<Player> winner = std::nullopt;
};