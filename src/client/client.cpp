#include "Client.h"

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#include <websocketpp/common/memory.hpp>

#include <iostream>
#include <memory>
#include <thread>

#include <client/ClientTypes.h>
#include <server/ConnectionMetadata.h>

Client::Client() {
    clear_access_channels(websocketpp::log::alevel::all);
    clear_error_channels(websocketpp::log::elevel::all);

    init_asio();
    start_perpetual();

    set_fail_handler([this](ConnectionHdl hdl) { failHandler(hdl); });

    set_message_handler(
        [this](ConnectionHdl hdl, MessagePtr msg) { messageHandler(hdl, std::move(msg)); });

    set_open_handler([this](ConnectionHdl hdl) { openHandler(hdl); });
}

void Client::failHandler(ConnectionHdl hdl) {
    auto botIter = m_botList.find(hdl);
    if (botIter == m_botList.end()) {
        std::cerr << "Fail handler: bot and associated connection not found.\n";
        return;
    }

    auto const &bot = botIter->second;
    auto uriPtr = bot->m_metadata->getUri();
    std::cerr << std::format("Connection to {:s} failed.\n", uriPtr->str());
}

void Client::openHandler(ConnectionHdl hdl) {
    auto botIter = m_botList.find(hdl);
    if (botIter == m_botList.end()) {
        std::cerr << "Open handler: bot and associated connection not found.\n";
        return;
    }
    auto bot = botIter->second;

    std::cout << std::format("Connection for {:s} opened.\n", bot->m_name);
    bot->sendRegistrationRequest();
    // bot->sendNewGameRequest();
}

void Client::messageHandler(ConnectionHdl hdl, MessagePtr msg) {
    auto botIter = m_botList.find(hdl);
    if (botIter == m_botList.end()) {
        std::cerr << "Message handler: bot and associated connection not found.\n";
        return;
    }

    auto const &bot = botIter->second;
    bot->processMessage(std::move(msg));
}

std::shared_ptr<Bot> Client::makeNewBot(std::string name, std::string const &uri) {
    websocketpp::lib::error_code ec;
    ClientConnectionPtr conPtr = get_connection(uri, ec);
    if (ec) {
        std::cout << std::format("Connectiom initialization error: {:s}.\n", ec.message());
        return nullptr;
    }

    auto uriPtr = conPtr->get_uri();

    auto handle = conPtr->get_handle();
    MetadataPtr metadataPtr = make_shared<ConnectionMetadata>(
        handle, ConnectionMetadata::Status::Connecting, uriPtr);

    connect(conPtr);

    auto bot = std::make_shared<Bot>(Bot(Bot::Params{.name = std::move(name),
                                                     .hdl = handle,
                                                     .metadata = metadataPtr,
                                                     .endpoint = shared_from_this()}));

    m_botList.insert(std::make_pair(handle, bot));
    return bot;
}

int main() {

    auto endpoint = std::make_shared<Client>();

    auto bot1 = endpoint->makeNewBot("Nika", "ws://localhost:6359");

    auto bot2 = endpoint->makeNewBot("Matic", "ws://localhost:6359");

    auto bot3 = endpoint->makeNewBot("Klara", "ws://localhost:6359");

    endpoint->run();
}