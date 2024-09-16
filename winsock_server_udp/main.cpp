
#include <iostream>
#include "WSServerUDP.hpp"

int main() {
    std::cout << "=================== SERVER UDP ===================\n";
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }


    try {
        WSServerUDP wssUDP(DEFAULT_PORT);
        std::string msgToSend;
        std::vector<char> receivedMsg;
        std::ostream_iterator<char> outIt(std::cout, "");
        while (true) {
            try {
                // Read from client
                receivedMsg = wssUDP.read();
                std::cout << "received message from CLIENT:\t";
                std::copy(receivedMsg.begin(), receivedMsg.end(), outIt);
                std::cout << '\n';
#if 0
                // Send message to client
                std::cout << "message to CLIENT:\t";
                std::getline(std::cin, msgToSend);
                if (msgToSend.empty())
                    break;
                wss.send(msgToSend.c_str());
#endif
            }
            catch (WSServerUDP::SocketError& err) {
                std::cout << err.what() << '\n';
                
            }
        }
    }
    catch (WSServerUDP::SocketError& err) {
        std::cout << err.what() << '\n';
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;

}