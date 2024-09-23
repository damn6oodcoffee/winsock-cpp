#pragma once
#define WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>
#include <iostream>
//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "../WSGeneral.hpp"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


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

	void send(const std::string& msg) {
		int iResult{
			sendto(
				connectSocket,
				msg.c_str(),
				msg.size(),
				0,
				ptr->ai_addr,
				ptr->ai_addrlen
			)
		};
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			throw SocketError{ "send failed" };
		}
	}

	std::vector<char> waitForACK() {
		int iTimeout = 1600;
		int iRet = setsockopt(connectSocket,
			SOL_SOCKET,
			SO_RCVTIMEO,
			/*
			reinterpret_cast<char*>(&tv),
			sizeof(timeval) );
			*/
			(const char*)&iTimeout,
			sizeof(iTimeout));
		sockaddr receiveAddr;
		int receiveLen{ sizeof(receiveAddr) };
		int iResult = recvfrom(connectSocket, &recvbuf[0], recvbuflen, 0, &receiveAddr, &receiveLen);
		if (iResult == SOCKET_ERROR) {
			printf("receive ack failed with error: %d\n", WSAGetLastError());
			throw SocketError{ "receive ack failed" };
		}
		return recvbuf;
	}

	std::vector<char> sendAndWaitForACK(const std::string& msg) {
		send(msg);
		auto received = waitForACK();
		return received;
	}

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

	void tryToConnect(addrinfo* result) {
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

		std::vector<char> addrStrBuf;
		addrStrBuf.resize(INET6_ADDRSTRLEN);
		auto dst = inet_ntop(ptr->ai_family, getInAddr(reinterpret_cast<sockaddr*>(ptr->ai_addr)), &addrStrBuf[0], addrStrBuf.size());
		std::cout << "connecting to " << dst << '\n';
	}

};


inline void sendUntilACK(WSClientUDP& wsc, const std::string& msg) {
	std::ostream_iterator<char> outIt(std::cout, "");
	while (true) {
		try {
			wsc.send(msg.c_str());
			auto response = wsc.waitForACK();
			std::cout << "received message from SERVER:\t";
			std::copy(response.begin(), response.end(), outIt);
			std::cout << '\n';
			std::cout << '\n';
			break;
		}
		catch (SocketError& err) {
			std::cout << err.what() << '\n';
			std::cout << "trying to resend...\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}
}