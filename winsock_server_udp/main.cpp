
#include <iostream>
#include <sstream>
#include "WSServerUDP.hpp"

int main() {
    std::cout << "=================== SERVER UDP ===================\n";

    WinSockStartupWrapper wsStartup;
    auto [addressIn, portIn] = addressAndPortInput();

    try {
        WSServerUDP wssUDP(addressIn, portIn);
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
            } catch (SocketError& err) {
                std::cout << err.what() << '\n';
            }
        }
    }
    catch (SocketError& err) {
        std::cout << err.what() << '\n';
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;

}