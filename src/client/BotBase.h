#ifndef BOT_BASE_H
#define BOT_BASE_H

#include "IBot.h"

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>

#include <client/Client_fwd.h>
#include <client/IBot.h>

struct BotBase : public IBot {
    friend class Client;

    virtual ~BotBase() = default;

    struct Params {
        std::string name;
        ConnectionMetadata metadata;
        std::shared_ptr<Client> endpoint;
    };

    using GameId = GameManager::GameId;

    BotBase(Params p);

    void sendProtoMessage(google::protobuf::Message const &message) override;
    void sendRegistrationRequest() override;
    void sendNewGameRequest() override;
    void processNewGameResponse(game_proto::NewGameResponse const &response) override;
    void processMessage(MessagePtr msg) override;

    ConnectionMetadata const &getConnectionMetadata() const override { return m_metadata; }
    std::string const &getName() const override { return m_name; }

    std::string m_name;
    ConnectionMetadata m_metadata;
    std::unordered_set<GameId> m_games;
    std::shared_ptr<Client> m_endpoint;
};

#endif