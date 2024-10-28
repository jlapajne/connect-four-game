#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

#include <websocketpp/server.hpp>

struct Player {
    std::string const username;
    std::string const displayName;
    websocketpp::connection_hdl hdl;
    std::uint32_t rating = 1500;

    bool operator==(Player const &other) const {
        return username == other.username && displayName == other.displayName;
    }
};

#endif