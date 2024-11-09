#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

#include <server/ConnectionMetadata.h>
#include <server/ServerTypes.h>
#include <websocketpp/server.hpp>

// Interface class for a player. This makes it possible to add
// different types of players.
struct IPlayer {

    virtual std::string const &getUsername() const = 0;
    virtual std::string const &getDisplayName() const = 0;

    virtual void setRating(std::uint32_t) = 0;
    virtual std::uint32_t getRating() const = 0;

    virtual void setId(ConnectionId) = 0;
    virtual ConnectionId getId() const = 0;

    virtual ConnectionId getConnection() const = 0;
    virtual void setConnection(ConnectionId hdl) = 0;

    virtual ~IPlayer() = default;
};

class Player : public IPlayer {
  public:
    struct Params {
        std::string username;
        std::string displayName;
        ConnectionId id;
    };

    Player(Params params)
        : m_username(std::move(params.username)), m_displayName(std::move(params.displayName)),
          m_id(std::move(params.id)) {}

    // clang-format off
    std::string const &getUsername() const override { return m_username; }
    std::string const &getDisplayName() const override { return m_displayName; }
    std::uint32_t getRating() const override { return m_rating; }
    ConnectionId getId() const override { return m_id; }


    void setRating(std::uint32_t rating) override { m_rating = rating; }
    void setId(ConnectionId hdl) override { m_id = std::move(hdl); }

    ConnectionId getConnection() const override { return m_id; }
    void setConnection(ConnectionId hdl) override { m_id = std::move(hdl); }

    // clang-format on

  private:
    std::string m_username;
    std::string m_displayName;
    ConnectionId m_id;
    std::uint32_t m_rating = 1500;
};

using PlayerHdl = IPlayer *;
using PlayerPtr = std::shared_ptr<IPlayer>;

#endif