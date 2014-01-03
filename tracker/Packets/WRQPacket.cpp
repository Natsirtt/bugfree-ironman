#include "WRQPacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

WRQPacket::WRQPacket(std::string filename, int partition) : mFileName(filename), mPartition(partition) {
}

WRQPacket::WRQPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        delete[] data;
        throw std::runtime_error("Erreur lors du traitement d'un paquet RRQ\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mPartition = ntohl(*partNb);

}

WRQPacket::~WRQPacket() {

}

unsigned int WRQPacket::getOpcode() {
    return WRQ;
}

int WRQPacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(int);
}

char* WRQPacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    *partNb = htonl(mPartition);

    return data;
}

void WRQPacket::exec(std::string adresse) {
    // TODO
}

std::string WRQPacket::getName() {
    return mFileName;
}

int WRQPacket::getPartitionNb() {
    return mPartition;
}
