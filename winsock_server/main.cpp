
#include <iostream>
#include "WSServer.hpp"

int main() {
    std::cout << "=================== SERVER ===================\n";

    WinSockStartupWrapper wsStartup;
    auto [addressIn, portIn] = addressAndPortInput();

    try {
        WSServer wss(addressIn, portIn);
        wss.accept();
        std::string msgToSend;
        std::vector<char> receivedMsg;
        std::ostream_iterator<char> outIt(std::cout, "");
        while (true) {
            try {
                // Read from client
                receivedMsg = wss.read();
                std::cout << "received message from CLIENT:\t";
                std::copy(receivedMsg.begin(), receivedMsg.end(), outIt);
                std::cout << '\n';
                // Send message to client
                std::cout << "message to CLIENT:\t";
                std::getline(std::cin, msgToSend);
                if (msgToSend.empty())
                    break;
                wss.send(msgToSend.c_str());
            } catch (SocketError& err) {
                std::cout << err.what() << '\n';
                wss.accept();
            }
        }
    } catch (SocketError& err) {
        std::cout << err.what() << '\n';
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}