#include "NetworkTranslator.hpp"

#include <arpa/inet.h>
#include "Opcode.hpp"
#include "Packets/RRQPacket.hpp"
#include "Packets/WRQPacket.hpp"
#include "Packets/DataPacket.hpp"
#include "Packets/ACKPacket.hpp"

NetworkTranslator::NetworkTranslator(SocketUDP* socket) : mSocket(socket) {

}

IPacket* NetworkTranslator::readPacket(std::string& adresse, int* port, int timeout) {

    char data[512]; // TODO calculer la taille du buffer
    int sizeRead = 0;
    if ((sizeRead = mSocket->read(data, 512, adresse, port, timeout)) <= 0) {
        return NULL;
    }


    int opcode = ntohl(*(int*) data);

    IPacket* packet = NULL;

    // TODO Traduire les paquets
    switch (opcode) {
    case RRQ:
        packet = new RRQPacket(data, sizeRead);
    case WRQ:
        packet = new WRQPacket(data, sizeRead);
    case DATA:
        packet = new DataPacket(data, sizeRead);
    case ACK:
        packet = new ACKPacket(data, sizeRead);
    }

    return packet;
}
