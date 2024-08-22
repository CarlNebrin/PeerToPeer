#include <iostream>

#ifdef _WIN32 // Check if compiling for Windows

#include <winsock2.h>
#include <ws2tcpip.h>

#else // Linux

// Linux-specific includes
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstring> // Include for strerro

// Define Windows-specific types if needed
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif

#include <bitset>
#include <climits>
#include <cstdint>
#include <cstring>
#include <thread>  // for std::this_thread::sleep_for
#include <chrono>  // for std::chrono::seconds

#define DEFAULT_PORT 7777