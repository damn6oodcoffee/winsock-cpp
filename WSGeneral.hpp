#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


struct WinSockStartupWrapper {
	WinSockStartupWrapper() {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			throw std::runtime_error("WinSock start up failed");
		}
	}
	~WinSockStartupWrapper() {
		WSACleanup();
	}
};


struct SocketError : public std::runtime_error {
	SocketError(const std::string& error) : std::runtime_error(error) {}
};


inline void* getInAddr(struct sockaddr* psa) {
	if (psa->sa_family == AF_INET) {
		return &reinterpret_cast<sockaddr_in*>(psa)->sin_addr;
	}
	return &reinterpret_cast<sockaddr_in6*>(psa)->sin6_addr;
}

std::pair<std::string, std::string> addressAndPortInput() {
	std::string addressIn, portIn;
	std::cout << "enter server address: ";
	std::getline(std::cin, addressIn);
	std::cout << "\nenter server port: ";
	std::getline(std::cin, portIn);
	std::cout << "\n";

	std::stringstream ss(addressIn);
	ss >> addressIn;
	ss.clear();
	ss.str(portIn);
	ss >> portIn;

	if (addressIn.empty())
		addressIn = "localhost";
	if (portIn.empty())
		portIn = DEFAULT_PORT;

	std::cout << addressIn << '\n';
	std::cout << portIn << '\n';
	return { addressIn, portIn };
}
