#pragma once
#define WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


class WSClientUDP {
public:

	WSClientUDP()
		: connectSocket{ INVALID_SOCKET }
		, recvbuflen{ DEFAULT_BUFLEN }
		, ptr{ nullptr }
	{
		recvbuf.resize(recvbuflen);
	}

	WSClientUDP(const std::string& address, const std::string& port)
		: WSClientUDP()
	{
		connect(address, port);
	}

	~WSClientUDP() {
		if (connectSocket != INVALID_SOCKET)
			closesocket(connectSocket);
	}

	void connect(const std::string& address, const std::string& port) {
		auto result{ resolveAddressAndPort(address, port) };
		tryToConnect(result);
	}

	void send(const char* msg) {
		int iResult{
			sendto(
				connectSocket,
				msg,
				static_cast<int>(strlen(msg)),
				0,
				ptr->ai_addr,
				ptr->ai_addrlen
			)
		};/*
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			throw SocketError{ "send failed" };
		}*/
	}

	struct SocketError : public std::runtime_error {
		SocketError(const std::string& error) : std::runtime_error(error) {}
	};
private:
	SOCKET connectSocket;
	int recvbuflen;
	addrinfo* ptr;
	std::vector<char> recvbuf;

	addrinfo* resolveAddressAndPort(const std::string& address, const std::string& port) {
		addrinfo* result{ nullptr };
		addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		// Resolve the server address and port
		int iResult{ getaddrinfo(address.c_str(), port.c_str(), &hints, &result)};
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			throw SocketError{ "getaddrinfo failed" };
		}
		return result;
	}

	addrinfo* tryToConnect(addrinfo* result) {
		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
			// Create a SOCKET for connecting to server
			connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (connectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				continue;
			}
			break;
		}
		if (ptr == nullptr) {
			printf("failed to create socket: %ld\n", WSAGetLastError());
			throw SocketError{ "failed to create socket: " };
		}
	}

};