#ifndef IBOT_H
#define IBOT_H

#include <string>
#include <type_traits>
#include <unordered_set>

#include <game.pb.h>
#include <google/protobuf/message.h>

#include <client/ClientTypes.h>
#include <server/ConnectionMetadata.h>
#include <server/GameManager.h>

// Interface class for different types of bots.

struct IBot {
  public:
    using GameId = GameManager::GameId;

    virtual ~IBot() = default;

    virtual void sendProtoMessage(google::protobuf::Message const &message) = 0;
    virtual void sendRegistrationRequest() = 0;
    virtual void sendNewGameRequest() = 0;
    virtual void processNewGameResponse(game_proto::NewGameResponse const &response) = 0;
    virtual void processMessage(MessagePtr msg) = 0;

    virtual void sendMoveRequest(GameId const &gameId, std::uint32_t columnidx) = 0;
    virtual void sendFirstMoveRequest(GameId const &gameId) = 0;

    virtual void
    processAvailableMovesResponse(game_proto::AvailableMovesResponse const &response) = 0;

    virtual ConnectionMetadata const &getConnectionMetadata() const = 0;

    virtual std::string const &getName() const = 0;
};

template <typename BotType>
concept BotConcept = std::is_base_of_v<IBot, BotType>;

#endif