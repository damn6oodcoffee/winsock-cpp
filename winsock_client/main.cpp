

#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>
//#include "../WSGeneral.hpp"
#include "WSClient.hpp"



int main() {

    std::cout << "=================== CLIENT ===================\n";
    
    WinSockStartupWrapper wsStartup;
    auto [addressIn, portIn] = addressAndPortInput();

    try {
        ReconnectableWSClient ReconnWSC(addressIn, portIn);
        std::string msgToSend;
        std::vector<char> receivedMsg;
        std::ostream_iterator<char> outIt(std::cout, "");
        while (true) {
            std::cout << "enter your message to SERVER:\t";
            std::getline(std::cin, msgToSend);
            if (msgToSend.empty())
                break;
            ReconnWSC.send(msgToSend.c_str());
            receivedMsg = ReconnWSC.read();
            std::cout << "received message from SERVER:\t";
            std::copy(receivedMsg.begin(), receivedMsg.end(), outIt);
            std::cout << '\n';
        }

    } catch (SocketError& err) {
        std::cout << err.what() << '\n';
        return 1;
    }
    return 0;
}