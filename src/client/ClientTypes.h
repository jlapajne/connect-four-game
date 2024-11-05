#ifndef CLIENT_TYPES_H
#define CLIENT_TYPES_H

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

using ClientEndpoint = websocketpp::client<websocketpp::config::asio_client>;
using ClientConnectionPtr = ClientEndpoint::connection_ptr;

#endif