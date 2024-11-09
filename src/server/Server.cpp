#include <server/ConnectionMetadata.h>
#include <server/Server.h>

// clang-format off
Server::Server(Params params) : 
    Server::server<websocketpp::config::asio>(), 
    m_logic(std::make_unique<ServerLogic>(this)),
    m_threadPool(params.maxTaskThreads) {
    // clang-format on

    auto _1 = std::placeholders::_1;
    auto _2 = std::placeholders::_2;

    this->set_message_handler(std::bind(&Server::onMessage, this, _1, _2));
    this->set_open_handler(std::bind(&Server::onConnectionOpened, this, _1));
    this->set_close_handler(std::bind(&Server::onConnectionClosed, this, _1));

    this->set_access_channels(websocketpp::log::alevel::all);
    this->set_error_channels(websocketpp::log::elevel::all);

    this->init_asio();

    std::cout << "Listening on port " << params.port << std::endl;
    this->listen(params.port);
    this->start_accept();
}

ConnectionPtr Server::getConnectionPtr(ConnectionHdl hdl) { return get_con_from_hdl(hdl); }

void Server::onMessage(ConnectionHdl hdl, MessagePtr msg) {

    // According to documentation, connection pointers can only be used within handler methods:
    // https://docs.websocketpp.org/md_tutorials_utility_client_utility_client.html
    // Although, this is a handler method, further execution is deffered to asio thread pool.
    // According to this: https://github.com/zaphoyd/websocketpp/issues/62, it is possible to
    // send connection pointer to asio thread (from thread pool) and use it from there. This is
    // what we do.

    ConnectionPtr ptr = getConnectionPtr(hdl);
    asio::post(m_threadPool.get_executor(),
               [self = shared_from_this(), msg = std::move(msg), id = getConnectionId(ptr)]() {
                   self->m_logic->decodeAndProcessRequest(id, std::move(msg));
               });
}

void Server::onConnectionClosed(ConnectionHdl hdl) {

    auto ptr = getConnectionPtr(hdl);
    auto id = getConnectionId(ptr);

    auto connection = m_connections.find(id);
    if (connection != m_connections.end()) {
        connection->second.setStatus(ConnectionMetadata::Status::Disconnected);
    } else {
        std::cerr << "Connection that is supposed to be closed, not found!";
    }

    asio::post(m_threadPool.get_executor(),
               [self = shared_from_this(), id = getConnectionId(ptr)]() {
                   self->m_logic->onConnectionClosed(id);
               });
}

void Server::onConnectionOpened(ConnectionHdl hdl) {

    auto connectionPtr = getConnectionPtr(hdl);
    auto connectionId = getConnectionId(connectionPtr);

    auto connection = m_connections.find(connectionId);
    if (connection != m_connections.end()) {
        // TODO(implement proper handling of such case).
        std::cerr << "Connection that is supposed to be opened, already exists!"
                     "Not add new connection to the list";
    }

    auto uri = connectionPtr->get_uri();

    auto [iter, success] = m_connections.emplace(std::make_pair(
        connectionId, ConnectionMetadata(hdl, ConnectionMetadata::Status::Connected, uri)));
    if (!success) {
        std::cerr << "Failed to establish connection to: " << uri->get_port() << std::endl;
        return;
    }

    std::cout << std::format("Connection opened to: {:d}.", uri->get_port()) << std::endl;
}

ConnectionMetadata::Status Server::getConnectionStatus(ConnectionId id) const {
    auto iter = m_connections.find(id);
    if (iter != m_connections.end()) {
        return iter->second.getStatus();
    }
    // If connection is not found in the list of connections, then it's disconnected.
    return ConnectionMetadata::Status::Disconnected;
}
