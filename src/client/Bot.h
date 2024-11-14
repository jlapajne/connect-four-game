#ifndef BOT_H
#define BOT_H

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#include <websocketpp/common/memory.hpp>

#include <cstdint>
#include <iostream>
#include <memory>
#include <thread>

#include <client/ClientTypes.h>
#include <server/ConnectionMetadata.h>
#include <server/GameManager.h>

#include <client/BotBase.h>
#include <client/Client_fwd.h>
#include <client/IBot.h>

enum class BotType : std::uint32_t { Random };

std::shared_ptr<IBot> makeNewBot(BotType type,
                                 std::string name,
                                 ConnectionMetadata metadata,
                                 std::shared_ptr<Client> endpoint);

class RandomBot : public BotBase {
    using Params = BotBase::Params;

    friend std::shared_ptr<IBot> makeNewBot(BotType type,
                                            std::string name,
                                            ConnectionMetadata metadata,
                                            std::shared_ptr<Client> endpoint);

  public:
    // Private constructor, because the class can only be constructed through the Client class.
    RandomBot(Params p);

    void sendMoveRequest(GameId const &gameId, std::uint32_t columnidx) override;

    void sendFirstMoveRequest(GameId const &gameId) override;

    void
    processAvailableMovesResponse(game_proto::AvailableMovesResponse const &response) override;
};

#endif