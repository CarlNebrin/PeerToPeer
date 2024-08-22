#include "server.hpp"

bool Server::hostServer() {
    #ifdef _WIN32
        WSADATA _wsaData;
        int _iResult;

        // Initialize Winsock
        _iResult = WSAStartup(MAKEWORD(2,2), &_wsaData);
        if (_iResult != 0) {
            std::cout<<"WSAStartup failed"<<std::endl;
        };
    #endif

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	
    #ifdef _WIN32
        // Windows non-blocking socket
        u_long _iMode = 1;
        ioctlsocket(serverSocket, FIONBIO, &_iMode);
    #else
        // Linux non-blocking socket
        int _flags = fcntl(serverSocket, F_GETFL, 0);
        fcntl(serverSocket, F_SETFL, _flags | O_NONBLOCK);
    #endif

	if (serverSocket == INVALID_SOCKET) {
		std::cout<<"Failed to create socket, ERROR: "<<errno<<std::endl;
	};

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
	serverAddress.sin_port = htons(DEFAULT_PORT);

	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
		std::cout<<"Failed to bind to port, ERROR: "<<errno<<std::endl;
	}
    else {
        std::cout<<"Server successfully created."<<std::endl;
    };

	std::cout<<"Listening..."<<std::endl;
	int _listen = listen(serverSocket,10);
	if (_listen == SOCKET_ERROR) {
		std::cout<<"Listen fail."<<std::endl;
        return false;
	}
	else {
		std::cout<<"Listen success."<<std::endl;
        return true;
	};

    return true;
};

void Server::findClients() {
    sockaddr_in _clientAddress;

    #ifdef _WIN32
        int _clientSize = sizeof(_clientAddress);
        SOCKET _clientSocket = accept(serverSocket, (struct sockaddr*)&_clientAddress, &_clientSize);
    #else
        socklen_t _clientSize = sizeof(_clientAddress);
        SOCKET _clientSocket = accept4(serverSocket, (struct sockaddr*)&_clientAddress, &_clientSize, SOCK_NONBLOCK); // Linux non-blocking socket
    #endif

    if (_clientSocket != INVALID_SOCKET) { // Client connected
        // Convert client address to a string
        char _connectedIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &( _clientAddress.sin_addr), _connectedIp, INET_ADDRSTRLEN);

        std::cout << "New client connected! IP address: " << _connectedIp << std::endl;

        connectedClients.push_back((uint32_t)_clientSocket);
    };
};

bool Server::connectPeers(const int &_clientAIndex, const int &_clientBIndex) {
    // Get initial ack message from each peer to be sent to the other
    int32_t _clientAMsg = 0; // Message sent from client A, to be delivered to client B
    int32_t _clientBMsg = 0; // Message sent from client B, to be delivered to client A

    int _rcvResultA = 0; // Bytes received from client A
    int _rcvResultB = 0; // Bytes received from client B

    while (_rcvResultA < 1 || _rcvResultB < 1) {
        if (_rcvResultA < 1) {
            std::cout<<"Attempting to get message from client A..."<<std::endl;
            _rcvResultA = recv(connectedClients[_clientAIndex], (char*)&_clientAMsg, sizeof(_clientAMsg), 0);
            if (_rcvResultA > 0) {
                std::cout<<"Got message: "<<_clientAMsg<<" from client A!"<<std::endl;
            };
        };

        if (_rcvResultB < 1) {
            std::cout<<"Attempting to get message from client B..."<<std::endl;
            _rcvResultB = recv(connectedClients[_clientBIndex], (char*)&_clientBMsg, sizeof(_clientBMsg), 0);
            if (_rcvResultB > 0) {
                std::cout<<"Got message: "<<_clientBMsg<<" from client B!"<<std::endl;
            };
        };

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };

    // Send _clientAMsg to client B, and vice versa
    int _sendResultA = 0;
    int _sendResultB = 0;

    while (_sendResultA < 1 || _sendResultB < 1) {
        if (_sendResultA < 1) {
            std::cout<<"Attempting to send message from client B to client A..."<<std::endl;
            _sendResultA = send(connectedClients[_clientAIndex], (char*)&_clientBMsg, sizeof(_clientBMsg), 0);
            if (_sendResultA > 0) {
                std::cout<<"Message sent successfully sent to client A!"<<std::endl;
            };
        };
        
        if (_sendResultB < 1) {
            std::cout<<"Attempting to send message from client A to client B..."<<std::endl;
            _sendResultB = send(connectedClients[_clientBIndex], (char*)&_clientAMsg, sizeof(_clientAMsg), 0);
            if (_sendResultB > 0) {
                std::cout<<"Message sent successfully sent to client B!"<<std::endl;
            };
        };

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };


    return true;
};

void Server::update() {
    hostServer();

    // Limit frame rate
    const std::chrono::duration<float> _frameDuration(0.25f); // Update every 0.25 seconds

    bool _peersConnected = false;

    // Server loop
    while (true) {
        
        if ((int)connectedClients.size() < MAX_PEERS) {
            findClients();
        }
        // Handshake peers 0 and 1
        else if (!_peersConnected) {
            _peersConnected = connectPeers(0, 1);
        };

        // Sleep 0.1 second
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
};