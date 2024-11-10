
#include "BotBase.h"

#include <format>
#include <iostream>

#include <game.pb.h>

#include <client/Client.h>
#include <server/ConnectFourGame.h>
#include <server/RandomUtils.h>
#include <server/ServerTypes.h>

BotBase::BotBase(Params p)
    : m_name(std::move(p.name)), m_metadata(std::move(p.metadata)), m_endpoint(p.endpoint) {}

void BotBase::processMessage(MessagePtr msg) {
    game_proto::Response message;

    std::string const &payload = msg->get_payload();
    message.ParseFromArray(payload.data(), payload.size());

    if (message.has_registration_success_response()) {
        std::cout << std::format("{:s} registered successfully.\n", m_name);
        sendNewGameRequest();
    } else if (message.has_new_game_response()) {
        processNewGameResponse(*message.mutable_new_game_response());
    }
}

void BotBase::sendProtoMessage(google::protobuf::Message const &message) {

    auto messageSize = message.ByteSizeLong();
    std::vector<char> payload(messageSize);
    message.SerializeToArray(payload.data(), messageSize);
    m_endpoint->send(
        m_metadata.getHdl(), payload.data(), messageSize, websocketpp::frame::opcode::binary);
}

void BotBase::sendRegistrationRequest() {

    std::cout << "Sending registration request.\n";
    game_proto::Request request;
    auto &registrationRequest = *request.mutable_registration_request();

    auto &credentials = *registrationRequest.mutable_user_credentials();
    credentials.set_username(m_name);
    credentials.set_display_name(m_name);

    sendProtoMessage(request);
}

void BotBase::sendNewGameRequest() {
    std::cout << std::format("{:s} sent new game request.\n", m_name);
    game_proto::Request request;
    request.mutable_new_game_request();
    sendProtoMessage(request);
}

void BotBase::processNewGameResponse(game_proto::NewGameResponse const &response) {

    auto const &gameId = response.game_id();
    auto [gameIter, success] = m_games.insert(gameId);
    assert(success);

    std::cout << std::format("{:s} starting a new game against {:s} with rating {:d}.\n",
                             m_name,
                             response.opponent_display_name(),
                             response.opponent_rating());

    if (response.make_first_move()) {
        sendFirstMoveRequest(gameId);
    }
}
