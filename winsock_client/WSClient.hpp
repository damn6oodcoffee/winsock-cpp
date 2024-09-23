#pragma once

#define WIN32_LEAN_AND_MEAN

#include <vector>
#include <string>
#include <stdexcept>
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


class WSClient {
public:

	WSClient()
		: connectSocket{ INVALID_SOCKET }
		, recvbuflen{ DEFAULT_BUFLEN }
	{
		recvbuf.resize(recvbuflen);
	}

	WSClient(const std::string& address, const std::string& port)
		: WSClient()
	{
		connect(address, port);
	}

	~WSClient() { 
		if (connectSocket != INVALID_SOCKET)
			closesocket(connectSocket); 
	}

	void connect(const std::string& address, const std::string& port) {
		auto result{ resolveAddressAndPort(address, port) };
		tryToConnect(result);
	}

	void send(const std::string& msg) {
		int iResult = ::send(connectSocket, msg.c_str(), msg.size(), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			throw SocketError{ "send failed" };
		}
	}

	std::vector<char> read() {
		std::vector<char> received;
		int iResult = recv(connectSocket, &recvbuf[0], recvbuflen, 0);
		if (iResult == SOCKET_ERROR) {
			printf("recv failed with error: %d\n", WSAGetLastError());
			throw SocketError{ "recv failed" };
		}
		auto end = recvbuf.begin();
		std::advance(end, iResult);
		received.assign(recvbuf.begin(), end);
		return received;
	}


private:
	SOCKET connectSocket;
	int recvbuflen;
	std::vector<char> recvbuf;

	addrinfo* resolveAddressAndPort(const std::string& address, const std::string& port) {
		addrinfo* result{ nullptr };
		addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		int iResult{ getaddrinfo(address.c_str(), port.c_str(), &hints, &result) };
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			throw SocketError{ "getaddrinfo failed" };
		}
		return result;
	}

	void tryToConnect(addrinfo* result) {
		addrinfo* ptr{ nullptr };
		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (connectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				throw SocketError{ "connect socket failed" };
			}

			// Connect to server.
			int iResult = ::connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(connectSocket);
				connectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		if (ptr) {
			std::vector<char> addrStrBuf;
			addrStrBuf.resize(INET6_ADDRSTRLEN);
			auto dst = inet_ntop(ptr->ai_family, getInAddr(reinterpret_cast<sockaddr*>(ptr->ai_addr)), &addrStrBuf[0], addrStrBuf.size());
			std::cout << "connecting to " << dst << '\n';
		}
		freeaddrinfo(result);

		if (connectSocket == INVALID_SOCKET) {
			printf("Unable to connect to server!\n");
			throw SocketError("unable to connect to server.");
		}
	}

};


class ReconnectableWSClient {
public:
	ReconnectableWSClient()
		: m_wsc()
	{}

	ReconnectableWSClient(const std::string& address, const std::string& port)
		: ReconnectableWSClient()
	{
		connect(address, port);
	}

	void connect(const std::string& address, const std::string& port) {
		m_address = address;
		m_port = port;
		while (true) {
			try {
				m_wsc.connect(address, port);
				std::cout << "connection established.\n";
				break;
			}
			catch (SocketError& err) {
				std::cout << err.what() << '\n';
			}
		}
	}

	void send(const char* msg) {
		while (true) {
			try {
				m_wsc.send(msg);
				break;
			}
			catch (SocketError& err) {
				std::cout << err.what() << '\n';
				std::cout << "trying to reconnect..." << '\n';
				connect(m_address, m_port);
			}
		}
	}

	std::vector<char> read() {
		while (true) {
			try {
				auto receivedMsg = m_wsc.read();
				return receivedMsg;
				break;
			}
			catch (SocketError& err) {
				std::cout << err.what() << '\n';
				std::cout << "trying to reconnect..." << '\n';
				connect(m_address, m_port);
			}
		}
	}


private:
	WSClient m_wsc;
	std::string m_address;
	std::string m_port;
};