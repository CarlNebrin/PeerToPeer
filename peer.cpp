#include "peer.hpp"

bool Peer::connectToRelayServer(const std::string &_serverIp, const int &_serverPort) {
    #ifdef _WIN32
        WSADATA _wsaData;
        int _iResult;

        // Initialize Winsock
        _iResult = WSAStartup(MAKEWORD(2,2), &_wsaData);
        if (_iResult != 0) {
            std::cout<<"WSAStartup failed"<<std::endl;
        };
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #else
        serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); // Linux non-blocking socket
    #endif

	sockaddr_in _server;
	_server.sin_family = AF_INET;
	_server.sin_addr.s_addr = inet_addr(_serverIp.c_str());
	_server.sin_port = htons(_serverPort);

	int _connection = connect(serverSocket,(struct sockaddr*) &_server, sizeof(_server));
    #ifdef _WIN32
    if (_connection == SOCKET_ERROR) {
    #else
    if (_connection == SOCKET_ERROR && errno != EINPROGRESS) {
    #endif
        std::cout<<"Connection to relay server failed."<<std::endl;
		return false;
    }
    else {
        std::cout<<"Connection to relay server success!"<<std::endl;
    };

    #ifdef _WIN32 // Windows non-blocking socket
        u_long _iMode = 1;
        ioctlsocket(serverSocket, FIONBIO, &_iMode);
    #endif

	return true;
};

bool Peer::hostPeer(const int &_port) {
    #ifdef _WIN32
        WSADATA _wsaData;
        int _iResult;

        // Initialize Winsock
        _iResult = WSAStartup(MAKEWORD(2, 2), &_wsaData);
        if (_iResult != 0)
        {
            std::cout << "WSAStartup failed" << std::endl;
        };
    #endif

	hostSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
    #ifdef _WIN32
        // Windows non-blocking socket
        u_long _iMode = 1;
        ioctlsocket(hostSocket, FIONBIO, &_iMode);
    #else
        // Linux non-blocking socket
        int _flags = fcntl(hostSocket, F_GETFL, 0);
        fcntl(hostSocket, F_SETFL, _flags | O_NONBLOCK);
    #endif

	if (hostSocket == INVALID_SOCKET) {
		std::cout<<"Failed to create socket, ERROR: "<<errno<<std::endl;
	};

	hostAddress.sin_family = AF_INET;
	hostAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
	hostAddress.sin_port = htons(_port);

	if (bind(hostSocket, (struct sockaddr*)&hostAddress, sizeof(hostAddress)) == SOCKET_ERROR) {
		std::cout<<"Failed to bind to port, ERROR: "<<errno<<std::endl;
	}
    else {
        std::cout<<"Server successfully created."<<std::endl;
    };

    return true;
};

void Peer::startP2p() {
    std::cout<<"Enter relay server IP: "<<std::endl;
    std::string _serverIp;
    std::cin>>_serverIp;
    bool _connected = connectToRelayServer(_serverIp, DEFAULT_PORT);
    if (_connected) {
        int32_t _message = 0;

        std::cout<<"Enter a number to send: "<<std::endl;
        std::cin>>_message;

        // Send _message first to relay server, that we may determine a handshake
        std::cout<<"Attempting to send message to relay server..."<<std::endl;
        for (int _i = 0; _i < 1; _i++) {
            int _sendResult = 0;
            while (_sendResult < 1) {
                _sendResult = send(serverSocket, (char*)&_message, sizeof(_message), 0);
                // Sleep 0.1 second
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            };
        };
        std::cout<<"Sent message to relay server!"<<std::endl;

        // Receive other peer's _message from the relay server
        std::cout<<"Attempting to receive other peer's message to relay server..."<<std::endl;
        int32_t _rcvMsgRelay = 0;
        for (int _i = 0; _i < 1; _i++) {
            int _rcvResult = 0;
            while (_rcvResult < 1) {
                _rcvResult = recv(serverSocket, (char*)&_rcvMsgRelay, sizeof(_rcvMsgRelay), 0);
                // Sleep 0.1 second
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            };
        };
        std::cout<<"Received message of other peer from relay server!"<<std::endl;

        std::cout<<"Enter host port: "<<std::endl;
        int _hostPort = 0;
        std::cin>>_hostPort;
        bool _host = hostPeer(_hostPort);
        if (_host) {

            std::cout<<"Enter target peer IP: ";
            std::string _peerIp;
            std::cin>>_peerIp;
            std::cout<<"Enter target peer port: ";
            int _peerPort;
            std::cin>>_peerPort;
            int _sendsToOtherPeers = 0;
            const int _maxSends = 1000;

            struct sockaddr_in _targetPeerAddress;
            memset(&_targetPeerAddress, 0, sizeof(_targetPeerAddress));
            _targetPeerAddress.sin_family = AF_INET;
            _targetPeerAddress.sin_port = htons(_peerPort);  // Destination port
            _targetPeerAddress.sin_addr.s_addr = inet_addr(_peerIp.c_str());  // Destination IP
            socklen_t _targetSockLen = sizeof(_targetPeerAddress);

            // Send _message directly to other the peer a number of times to hole punch
            // One peer may successfully get the message earlier than the other, rather than break the while loop early
            // we will assume it requires at least _maxSends number of sends for a successful handshake between both peers to occur
            int32_t _rcvMsgFromPeer = 0; // The message received directly from the other peer
            while (_sendsToOtherPeers < _maxSends) {
                std::cout<<"Attempting to send message directly to other peer..."<<std::endl;
                // Send the message
                int _sendResult = sendto(hostSocket, (char*)&_message, sizeof(_message), 0, (struct sockaddr*)&_targetPeerAddress, sizeof(_targetPeerAddress));
                
                // Attempt to get the other peer's message
                int32_t _rcvMsg = 0;
                int _rcvResult = recvfrom(hostSocket, (char*)&_rcvMsg, sizeof(_rcvMsg), 0, (struct sockaddr*)&_targetPeerAddress, &_targetSockLen);
                if (_rcvResult > 0) {
                    _rcvMsgFromPeer = _rcvMsg;
                };

                if (_sendResult > 0) {
                    std::cout<<"Sent message directly to other peer successfully!"<<std::endl;
                    _sendsToOtherPeers += 1;
                };

                // Sleep 0.1 second
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            };
            
            // Verify that the message received directly from the other peer is the same message received from the relay server
            std::cout<<"Other peer message according to relay server: "<<_rcvMsgRelay<<std::endl;
            std::cout<<"Other peer message received directly from other peer: "<<_rcvMsgFromPeer<<std::endl;

            // Pause
            while (true) {
            };
        }
        else {
            std::cout<<"Failed to host."<<std::endl;
        };
    };
};