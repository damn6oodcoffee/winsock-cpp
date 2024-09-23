#pragma once

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <stdexcept>
#include <vector>

//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "../WSGeneral.hpp"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")



class WSServerUDP {
public:
	WSServerUDP(const std::string& address, const std::string& port)
		: listenSocket{ INVALID_SOCKET }
		, recvbuflen{ DEFAULT_BUFLEN }
	{
		recvbuf.resize(recvbuflen);
		auto result = resolveAddressAndPort(address, port);
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
		if (iResult == SOCKET_ERROR) {
			printf("receive ack failed with error: %d\n", WSAGetLastError());
			throw SocketError{ "receive ack failed" };
		}
		printf("Got %d bytes from %s\n", iResult, 
			inet_ntop(clientAddress.sa_family, getInAddr(&clientAddress), &addrStr[0], addrStr.size())
		);
		auto end = recvbuf.begin();
		std::advance(end, iResult);
		received.assign(recvbuf.begin(), end);

		// Send ACK response
		const char* response = "ACK";
		iResult = sendto(listenSocket, response, strlen(response), 0, &clientAddress, clientLen);
		if (iResult == SOCKET_ERROR) {
			printf("send ACK failed with error: %d\n", WSAGetLastError());
			throw SocketError{ "send ACK failed" };
		}
		return received;
	}


private:
	SOCKET listenSocket;
	int recvbuflen;
	std::vector<char> recvbuf;
	sockaddr clientAddress;

	addrinfo* resolveAddressAndPort(const std::string& address, const std::string& port) {
		addrinfo* result{ nullptr };
		addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		//hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		int iResult{ getaddrinfo(address.c_str(), port.c_str(), &hints, &result)};
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

		if (result) {
			std::vector<char> addrStrBuf;
			addrStrBuf.resize(INET6_ADDRSTRLEN);
			auto dst = inet_ntop(result->ai_family, getInAddr(reinterpret_cast<sockaddr*>(result->ai_addr)), &addrStrBuf[0], addrStrBuf.size());
			std::cout << "Set up socket at: " << dst << '\n';
		}

		freeaddrinfo(result);

	}
};