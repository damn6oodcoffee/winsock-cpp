


#include "WSClientUDP.hpp"




int main() {
    std::cout << "=================== CLIENT UDP ===================\n";
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    try {
        WSClientUDP wscUDP("127.0.0.1", DEFAULT_PORT);
        std::string msgToSend;
        while (true) {
            std::cout << "enter your message to SERVER:\t";
            std::getline(std::cin, msgToSend);
            if (msgToSend.empty())
                break;
            wscUDP.send(msgToSend.c_str());
        }
    } catch (WSClientUDP::SocketError& err) {
        std::cout << err.what() << '\n';
        WSACleanup();
        return 1;
    }
    
    WSACleanup();
    return 0;
}