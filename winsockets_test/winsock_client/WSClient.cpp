

#include <iostream>
#include <iterator>
#include <algorithm>
#include "WSClient.hpp"


/*
int __cdecl main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    const char* sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    const char* address = "localhost";
    iResult = getaddrinfo(address, DEFAULT_PORT, & hints, & result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            printf("Bytes received: %d\n", iResult);
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
*/


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
            } catch (WSClient::SocketError& err) {
                std::cout << err.what() << '\n';
            }
        }
    }

    void send(const char* msg) {
        while (true) {
            try {
                m_wsc.send(msg);
                auto receivedMsg = m_wsc.read();
                std::ostream_iterator<char> outIt(std::cout, "");
                std::cout << "received message:\t";
                std::copy(receivedMsg.begin(), receivedMsg.end(), outIt);
                std::cout << '\n';
                break;
            }
            catch (WSClient::SocketError& err) {
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


int main() {

    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    try {

#if 0
        WSClient wsc;
        wsc.connect("localhost", DEFAULT_PORT);

#endif // 0

        ReconnectableWSClient ReconnWSC("localhost", DEFAULT_PORT);

        std::string msgToSend;
        while (true) {
            std::cout << "enter your message:\t";
            std::getline(std::cin, msgToSend);
            if (msgToSend.empty())
                break;
#if 1
            ReconnWSC.send(msgToSend.c_str());
#else
            wsc.send(msgToSend.c_str());
            auto msg = wsc.read();
            std::ostream_iterator<char> outIt(std::cout, "");
            std::cout << "received message:\t";
            std::copy(msg.begin(), msg.end(), outIt);
            std::cout << '\n';
#endif
        }

    } catch (WSClient::SocketError& err) {
        std::cout << err.what() << '\n';
        return 1;
    }
    return 0;
}