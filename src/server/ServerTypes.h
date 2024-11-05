
#ifndef SERVER_TYPES_H
#define SERVER_TYPES_H

#include <stdexcept>

#include <game.pb.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/transport/asio/endpoint.hpp>

class GameException : public std::runtime_error {
  public:
    GameException(const std::string &msg, game_proto::ErrorCode errorCode)
        : std::runtime_error(msg), errorCode(errorCode) {}

    game_proto::ErrorCode getErrorCode() const { return errorCode; }

  private:
    game_proto::ErrorCode errorCode;
};

using ServerType = websocketpp::server<websocketpp::config::asio>;
using MessagePtr = ServerType::message_ptr;

#endif