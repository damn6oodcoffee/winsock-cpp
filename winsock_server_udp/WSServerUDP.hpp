#pragma once

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <stdexcept>
#include <vector>

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

void* getInAddr(struct sockaddr* psa) {
	if (psa->sa_family == AF_INET) {
		return &reinterpret_cast<sockaddr_in*>(psa)->sin_addr;
	}
	return &reinterpret_cast<sockaddr_in6*>(psa)->sin6_addr;
}

class WSServerUDP {
public:
	WSServerUDP(const std::string& port)
		: listenSocket{ INVALID_SOCKET }
		, recvbuflen{ DEFAULT_BUFLEN }
	{
		recvbuf.resize(recvbuflen);
		auto result = resolveAddressAndPort(port);
		setUpListenSocket(result);
	}

	~WSServerUDP() {
		if (listenSocket != INVALID_SOCKET)
			closesocket(listenSocket);
	}

	void send(const char* msg) {
		//int iResult{  }
	}

	std::vector<char> read() {
		std::vector<char> received;
		std::vector<char> addrStr;
		addrStr.resize(INET6_ADDRSTRLEN);
		int clientLen = sizeof(clientAddress);
		int iResult{ recvfrom(
			listenSocket,
			&recvbuf[0],
			recvbuflen,
			0,
			&clientAddress,
			&clientLen
		) };
		printf("Got %d bytes from %s\n", iResult, 
			inet_ntop(clientAddress.sa_family, getInAddr(&clientAddress), &addrStr[0], addrStr.size())
		);
		auto end = recvbuf.begin();
		std::advance(end, iResult);
		received.assign(recvbuf.begin(), end);
		return received;
	}

	struct SocketError : public std::runtime_error {
		SocketError(const std::string& error) : std::runtime_error(error) {}
	};

private:
	SOCKET listenSocket;
	int recvbuflen;
	std::vector<char> recvbuf;
	sockaddr clientAddress;

	addrinfo* resolveAddressAndPort(const std::string& port) {
		addrinfo* result{ nullptr };
		addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		int iResult{ getaddrinfo(nullptr, port.c_str(), &hints, &result) };
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
		// Setup listening socket
		int iResult = bind(listenSocket, result->ai_addr, result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			listenSocket = INVALID_SOCKET;
			throw SocketError("bind failed.");
		}

		freeaddrinfo(result);

	}


};