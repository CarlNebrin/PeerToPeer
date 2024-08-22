#ifndef SERVER_HPP
#define SERVER_HPP

#include "network.hpp"
#include <vector>

#define MAX_PEERS 2

struct Server {
    uint32_t serverSocket;
    sockaddr_in serverAddress;
    std::vector<uint32_t> connectedClients;

    bool hostServer();
    void findClients();
    bool connectPeers(const int& _clientAIndex, const int& _clientBIndex);
    void update();
};

#endif