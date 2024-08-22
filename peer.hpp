#ifndef PEER_HPP
#define PEER_HPP

    #include "network.hpp"

    struct Peer {
        uint32_t hostSocket = 0; // The peer's hosting socket
        sockaddr_in hostAddress; // The peer's hosting address

        uint32_t serverSocket = 0; // The relay server's socket
        sockaddr_in serverAddress; // The relay server's address

        bool connectToRelayServer(const std::string& _serverIp, const int& _serverPort);
        bool hostPeer(const int& _port);

        void startP2p();
    };


#endif