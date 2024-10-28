#include <server/Server.h>

// clang-format off
Server::Server(Params params) : 
    Server::server<websocketpp::config::asio>(), 
    m_logic(std::make_unique<ServerLogic>(shared_from_this())),
    m_threadPool(params.maxTaskThreads) {
    // clang-format on

    auto _1 = std::placeholders::_1;
    auto _2 = std::placeholders::_2;

    this->set_message_handler(std::bind(&Server::onMessage, this, _1, _2));
    this->set_access_channels(websocketpp::log::alevel::all);
    this->set_error_channels(websocketpp::log::elevel::all);

    this->init_asio();
    this->listen(params.port);
    this->start_accept();
}

void Server::onMessage(ConnectionPtr hdl, MessagePtr msg) {
    asio::post(m_threadPool.get_executor(),
               [self = shared_from_this(), msg = std::move(msg), hdl = std::move(hdl)]() {
                   self->m_logic->decodeAndProcessRequest(std::move(hdl), std::move(msg));
               });

    // logic.decodeAndProcessRequest(hdl, msg);
}