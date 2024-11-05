

#include <iostream>

#include <game.pb.h>

#include <client/Client.h>
#include <server/ServerTypes.h>

Bot::Bot(Params p)
    : m_name(std::move(p.name)), m_hdl(p.hdl), m_metadata(std::move(p.metadata)),
      m_endpoint(p.endpoint) {}

void Bot::processMessage(MessagePtr msg) {
    game_proto::Response message;
    message.ParseFromString(msg->get_payload());

    if (message.has_registration_success_response()) {
        std::cout << std::format("{:s} registered successfully.\n", m_name);
        sendNewGameRequest();
    }
}

void Bot::sendRegistrationRequest() {

    std::cout << "Sending registration request.\n";
    game_proto::Request request;
    auto &registrationRequest = *request.mutable_registration_request();

    auto &credentials = *registrationRequest.mutable_user_credentials();
    credentials.set_username(m_name);
    credentials.set_display_name(m_name);

    m_endpoint->send(m_hdl, request.SerializeAsString(), websocketpp::frame::opcode::text);
}

void Bot::sendNewGameRequest() {
    std::cout << std::format("{:s} sent new game request.\n", m_name);
    game_proto::Request request;
    request.mutable_new_game_request();
    m_endpoint->send(m_hdl, request.SerializeAsString(), websocketpp::frame::opcode::text);
}