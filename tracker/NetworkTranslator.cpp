#include "NetworkTranslator.hpp"

#include <iostream>
#include <arpa/inet.h>
#include "Opcode.hpp"
#include "Packets/RRQPacket.hpp"
#include "Packets/WRQPacket.hpp"
#include "Packets/DataPacket.hpp"
#include "Packets/ACKPacket.hpp"
#include "Packets/AlivePacket.hpp"
#include "Packets/FileRequestPacket.hpp"
#include "Packets/FileAnswerPacket.hpp"
#include "Packets/FileUpdatePacket.hpp"

NetworkTranslator::NetworkTranslator(SocketUDP* socket) : mSocket(socket) {

}

// Taille max d'un paquet (Taille d'un DataPacket)
#define MAX_PACKET_SIZE 629

IPacket* NetworkTranslator::readPacket(std::string& adresse, int* port, int timeout) {
    char data[MAX_PACKET_SIZE];
    int sizeRead = 0;
    if ((sizeRead = mSocket->read(data, MAX_PACKET_SIZE, adresse, port, timeout)) <= 0) {
        return NULL;
    }

    int* opcodePtr = (int*) data;
    int opcode = ntohl(*opcodePtr);
    IPacket* packet = NULL;

    switch (opcode) {
    case RRQ:
        packet = new RRQPacket(data, sizeRead);
        break;
    case WRQ:
        packet = new WRQPacket(data, sizeRead);
        break;
    case DATA:
        packet = new DataPacket(data, sizeRead);
        break;
    case ACK:
        packet = new ACKPacket(data, sizeRead);
        break;
    case ALIVE:
        packet = new AlivePacket(data, sizeRead);
        break;
    case FILE_REQUEST:
        packet = new FileRequestPacket(data, sizeRead);
        break;
    case FILE_ANSWER:
        packet = new FileAnswerPacket(data, sizeRead);
        break;
    case FILE_UPDATE:
        packet = new FileUpdatePacket(data, sizeRead);
        break;
    }

    return packet;
}
