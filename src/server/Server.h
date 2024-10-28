#ifndef SERVER_H
#define SERVER_H

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include <functional>
#include <iostream>

#include <asio/thread_pool.hpp>
#include <server/ServerLogic.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <websocketpp/extensions/permessage_deflate/enabled.hpp>

class Server : public websocketpp::server<websocketpp::config::asio>,
               public std::enable_shared_from_this<Server> {

  public:
    struct Params {
        std::uint32_t port = 9000;
        std::size_t maxTaskThreads = 1;
    };

    using ConnectionPtr = websocketpp::connection_hdl;

    Server(Params params)
        : Server::server<websocketpp::config::asio>(), m_threadPool(params.maxTaskThreads) {

        auto _1 = std::placeholders::_1;
        auto _2 = std::placeholders::_2;

        this->set_message_handler(std::bind(&Server::on_message, this, _1, _2));
        this->set_access_channels(websocketpp::log::alevel::all);
        this->set_error_channels(websocketpp::log::elevel::all);

        this->init_asio();
        this->listen(params.port);
        this->start_accept();
    }

  private:
    void on_message(ConnectionPtr hdl, Server::message_ptr msg);

  private:
    ServerLogic m_logic;
    asio::thread_pool m_threadPool;
};

void Server::on_message(ConnectionPtr hdl, Server::message_ptr msg) {

    // logic.decodeAndProcessRequest(hdl, msg);
}

#endif