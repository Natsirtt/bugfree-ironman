#include "RRQPacket.hpp"
#include "../Opcode.hpp"
#include "../ClientKnowledgeBase.hpp"
#include "../AnswerQueue.hpp"
#include "DataPacket.hpp"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>
#include <vector>

RRQPacket::RRQPacket(std::string filename, int partition, int firstBlock) : mFileName(filename), mPartition(partition), mFirstPacket(firstBlock) {
}

RRQPacket::RRQPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        throw std::runtime_error("Erreur lors du traitement d'un paquet RRQ\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mPartition = ntohl(*partNb);

    int* firstPacket = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(mPartition));
    mFirstPacket = htonl(*firstPacket);
}

RRQPacket::~RRQPacket() {

}

unsigned int RRQPacket::getOpcode() {
    return RRQ;
}

int RRQPacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(int) + sizeof(int);
}

char* RRQPacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    *partNb = htonl(mPartition);

    int* firstPacket = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(mPartition));
    *firstPacket = htonl(mFirstPacket);
    return data;
}

void RRQPacket::exec(std::string adresse) {
    std::cout << "exec RRQPacket" << std::endl;
    std::vector<char> block = ClientKnowledgeBase::get().getBlockData(mFileName, mPartition, mFirstPacket);
    IPacket* packet = new DataPacket(mFileName, mPartition, mFirstPacket, block.size(), block.data());
    AnswerQueue::get().sendToClient(packet, adresse);
}
