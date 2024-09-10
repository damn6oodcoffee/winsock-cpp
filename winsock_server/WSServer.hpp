#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
        
class WSServer {
public:
    WSServer(const std::string& port)
        : clientSocket{ INVALID_SOCKET }
        , listenSocket{ INVALID_SOCKET }
        , recvbuflen{ DEFAULT_BUFLEN }
    {
        recvbuf.resize(recvbuflen);
        auto result = resolveAddressAndPort(port);
        setUpListenSocket(result);
	}

    ~WSServer() {
        if (clientSocket != INVALID_SOCKET)
            closesocket(clientSocket);
        if (listenSocket != INVALID_SOCKET)
            closesocket(listenSocket);
    }
	
    int ijIo = 0;

	void accept() {
        std::cout << "Listening for client...\n";
        // Accept a client socket
        clientSocket = ::accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            throw SocketError("accept failed.");
        }
        std::cout << "Client accepted!\n";
	}

    void send(const char* msg) {
        int iResult = ::send(clientSocket, msg, static_cast<int>(strlen(msg)), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            throw SocketError{ "send failed" };
        }
    }

    std::vector<char> read() {
        std::vector<char> received;
        int iResult = recv(clientSocket, &recvbuf[0], recvbuflen, 0);
        if (iResult == SOCKET_ERROR) {
            printf("recv failed with error: %d\n", WSAGetLastError());
            throw SocketError{ "recv failed" };
        }
        auto end = recvbuf.begin();
        std::advance(end, iResult);
        received.assign(recvbuf.begin(), end);
        return received;
    }

    struct SocketError : public std::runtime_error {
        SocketError(const std::string& error) : std::runtime_error(error) {}
    };

private:
    SOCKET clientSocket;
    SOCKET listenSocket;
    int recvbuflen;
    std::vector<char> recvbuf;

	addrinfo* resolveAddressAndPort(const std::string& port) {
		addrinfo* result{ nullptr };
		addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		int iResult{ getaddrinfo(nullptr, port.c_str(), &hints, &result)};
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			throw SocketError{ "getaddrinfo failed" };
		}
		return result;
	}

	void setUpListenSocket(addrinfo* result) {
        // Create a SOCKET for the server to listen for client connections.
        listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (listenSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            throw SocketError("socket failed.");
        }

        // Setup the TCP listening socket
        int iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            printf("bind failed with error: %d\n", WSAGetLastError());
            freeaddrinfo(result);
            listenSocket = INVALID_SOCKET;
            throw SocketError("bind failed.");
        }

        freeaddrinfo(result);

        iResult = ::listen(listenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            listenSocket = INVALID_SOCKET;
            throw SocketError("listen failed.");
        }

	}

};
