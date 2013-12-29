#include "NetworkTranslator.hpp"
#include "Opcode.hpp"
#include "Packets/RRQPacket.hpp"

NetworkTranslator::NetworkTranslator(SocketUDP* socket) : mSocket(socket) {

}

IPacket* NetworkTranslator::readPacket(std::string& adresse, int* port, int timeout) {

    char data[512]; // TODO calculer la taille du buffer
    if (mSocket->read(data, 512, adresse, port, timeout) <= 0) {
        return NULL;
    }

    int opcode = 0; // TODO récuperer l'opcode dans data

    IPacket* packet = NULL;

    // TODO Traduire les paquets
    switch (opcode) {
    case RRQ:
        packet = new RRQPacket(data);
    }

    return packet;
}
