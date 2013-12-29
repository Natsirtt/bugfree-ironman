#include "DataPacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

DataPacket::DataPacket(std::string filename, int partition, int blockNb, int blockSize, char* blockData)
                        : mFileName(filename), mPartition(partition), mBlockNb(blockNb), mBlockSize(blockSize), mBlockData(blockData) {
}

DataPacket::DataPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        delete[] data;
        throw std::runtime_error("Erreur lors du traitement d'un paquet RRQ\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mPartition = ntohl(*partNb);
    // TODO
    delete[] data;
}

DataPacket::~DataPacket() {

}

int DataPacket::getOpcode() {
    return DATA;
}

int DataPacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(int) + sizeof(int) + sizeof(int) + MAX_DATA_SIZE;
}

char* DataPacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    *partNb = htonl(mPartition);

    int* blockNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(mPartition));
    *blockNb = htonl(mBlockNb);
    // TODO
    return data;
}

void DataPacket::exec(std::string adresse) {
    // TODO
}
