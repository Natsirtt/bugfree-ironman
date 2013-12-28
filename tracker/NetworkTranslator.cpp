#include "NetworkTranslator.hpp"

NetworkTranslator::NetworkTranslator(SocketUDP* socket) : mSocket(socket) {

}

IPacket* NetworkTranslator::readPacket(std::string& adresse, int* port, int timeout) {
    // TODO Traduire les paquets
    return NULL;
}
