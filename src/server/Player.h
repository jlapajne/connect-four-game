#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

#include <server/ServerTypes.h>
#include <websocketpp/server.hpp>

struct IPlayer {

    virtual std::string const &getUsername() const = 0;
    virtual std::string const &getDisplayName() const = 0;

    virtual void setRating(std::uint32_t) = 0;
    virtual std::uint32_t getRating() const = 0;

    virtual void setHdl(websocketpp::connection_hdl) = 0;
    virtual websocketpp::connection_hdl getHdl() const = 0;

    virtual ConnectionPtr getConnection() const = 0;
    virtual void setConnection(websocketpp::connection_hdl hdl) = 0;

    virtual ~IPlayer() = default;
};

class Player : public IPlayer {
  public:
    struct Params {
        std::string username;
        std::string displayName;
        websocketpp::connection_hdl hdl;
    };

    Player(Params params)
        : m_username(std::move(params.username)), m_displayName(std::move(params.displayName)),
          m_hdl(std::move(params.hdl)) {}

    // clang-format off
    std::string const &getUsername() const override { return m_username; }
    std::string const &getDisplayName() const override { return m_displayName; }
    std::uint32_t getRating() const override { return m_rating; }
    websocketpp::connection_hdl getHdl() const override { return m_hdl; }


    void setRating(std::uint32_t rating) override { m_rating = rating; }
    void setHdl(websocketpp::connection_hdl hdl) override { m_hdl = std::move(hdl); }

    ConnectionPtr getConnection() const override { return m_hdl; }
    void setConnection(ConnectionPtr hdl) override { m_hdl = std::move(hdl); }

    // clang-format on

  private:
    std::string m_username;
    std::string m_displayName;
    ConnectionPtr m_hdl;
    std::uint32_t m_rating = 1500;
};

#endif