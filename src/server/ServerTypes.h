
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using BaseServerType = websocketpp::server<websocketpp::config::asio>;
using ConnectionPtr = websocketpp::connection_hdl;
using MessagePtr = BaseServerType::message_ptr;