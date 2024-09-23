

#include <sstream>
#include <chrono>
#include <thread>
#include "WSClientUDP.hpp"



int main() {
    std::cout << "=================== CLIENT UDP ===================\n";

    WinSockStartupWrapper wsStartup;
    auto [addressIn, portIn] = addressAndPortInput();

    try {
        WSClientUDP wscUDP(addressIn, portIn);
        std::string msgToSend;
        while (true) {
            std::cout << "enter your message to SERVER:\t";
            std::getline(std::cin, msgToSend);
            if (msgToSend.empty())
                break;
            sendUntilACK(wscUDP, msgToSend);
        }
    } catch (SocketError& err) {
        std::cout << err.what() << '\n';
        return 1;
    }
    
    return 0;
}