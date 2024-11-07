#ifndef CLIENT_H
#define CLIENT_H

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#include <websocketpp/common/memory.hpp>

#include <iostream>
#include <memory>
#include <set>
#include <thread>
#include <unordered_set>

#include <game.pb.h>
#include <google/protobuf/message.h>

#include <client/ClientTypes.h>
#include <server/ConnectionMetadata.h>
#include <server/GameManager.h>

class Bot;
class Client : public ClientEndpoint, public std::enable_shared_from_this<Client> {
  public:
    Client();

    std::shared_ptr<Bot> makeNewBot(std::string name, std::string const &port);

  private:
    void failHandler(ConnectionHdl hdl);
    void messageHandler(ConnectionHdl hdl, MessagePtr msg);
    void openHandler(ConnectionHdl hdl);

  private:
    using BotList =
        std::map<ConnectionHdl, std::shared_ptr<Bot>, std::owner_less<ConnectionHdl>>;
    BotList m_botList;
};

class Bot {

    friend class Client;

    using GameId = GameManager::GameId;

    struct Params {
        std::string name;
        ConnectionHdl hdl;
        MetadataPtr metadata;
        std::shared_ptr<Client> endpoint;
    };

    // Private constructor, because the class can only be constructed through the Client class.
    Bot(Params p);

    void sendProtoMessage(google::protobuf::Message const &message);

    void sendRegistrationRequest();
    void sendNewGameRequest();
    void sendFirstMoveRequest(GameId const &gameId);

    void processNewGameResponse(game_proto::NewGameResponse const &response);

    void processMessage(MessagePtr msg);

    std::string m_name;
    ConnectionHdl m_hdl;
    MetadataPtr m_metadata;
    std::unordered_set<GameId> m_games;
    std::shared_ptr<Client> m_endpoint;
};

#endif