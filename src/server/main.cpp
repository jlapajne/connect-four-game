#include "Server.h"

#include <memory>

int main(int argc, char **argv) {
    auto server = std::make_shared<Server>(Server::Params{.port = 9000, .maxTaskThreads = 1});
    server->run();
    return 0;
}