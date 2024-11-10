#ifndef CLIENT_H
#define CLIENT_H

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#include <websocketpp/common/memory.hpp>

#include <iostream>
#include <memory>
#include <thread>
#include <type_traits>
#include <unordered_set>

#include <game.pb.h>
#include <google/protobuf/message.h>

#include <client/Bot.h>
#include <client/ClientTypes.h>
#include <client/IBot.h>
#include <server/ConnectionMetadata.h>
#include <server/GameManager.h>

class Client : public ClientEndpoint, public std::enable_shared_from_this<Client> {
  public:
    Client();

    std::shared_ptr<IBot>
    makeBot(BotType type, std::string const &name, std::string const &port);

  private:
    void failHandler(ConnectionHdl hdl);
    void messageHandler(ConnectionHdl hdl, MessagePtr msg);
    void openHandler(ConnectionHdl hdl);

  private:
    using BotList =
        std::map<ConnectionHdl, std::shared_ptr<IBot>, std::owner_less<ConnectionHdl>>;
    BotList m_botList;
};

#endif