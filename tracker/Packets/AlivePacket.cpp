#include "AlivePacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

#include "../KnowledgeBase.hpp"

AlivePacket::AlivePacket() {
}

AlivePacket::AlivePacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        delete[] data;
        throw std::runtime_error("Erreur lors du traitement d'un paquet Alive\n");
    }
}

AlivePacket::~AlivePacket() {

}

unsigned int AlivePacket::getOpcode() {
    return ALIVE;
}

int AlivePacket::getSize() {
    return sizeof(int);
}

char* AlivePacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    return data;
}

void AlivePacket::exec(std::string adresse) {
    KnowledgeBase::get().getClient(adresse).alive();
}
