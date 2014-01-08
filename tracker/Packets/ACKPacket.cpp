#include "ACKPacket.hpp"
#include "../Opcode.hpp"
#include "../ClientKnowledgeBase.hpp"
#include "DataPacket.hpp"
#include "../AnswerQueue.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

ACKPacket::ACKPacket(std::string filename, int partition, int blockNb, int nextBlock)
                        : mFileName(filename), mPartition(partition), mBlockNb(blockNb), mNextBlock(nextBlock) {
}

ACKPacket::ACKPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        delete[] data;
        throw std::runtime_error("Erreur lors du traitement d'un paquet ACK\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mPartition = ntohl(*partNb);

    int* blockNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(mPartition));
    mBlockNb = htonl(*blockNb);

    int* nextBlock = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(mPartition) + sizeof(mBlockNb));
    mNextBlock = htonl(*nextBlock);

}

ACKPacket::~ACKPacket() {

}

unsigned int ACKPacket::getOpcode() {
    return ACK;
}

int ACKPacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(int) + sizeof(int) + sizeof(int);
}

char* ACKPacket::toData() {
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

    int* nextBlock = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(mPartition) + sizeof(mBlockNb));
    *nextBlock = htonl(mNextBlock);

    return data;
}

void ACKPacket::exec(std::string adresse) {
    std::cout << "exec ACKPacket" << std::endl;
    if (mNextBlock >= 0) {
        std::vector<char> block = ClientKnowledgeBase::get().getBlockData(mFileName, mPartition, mNextBlock);

        IPacket* packet = new DataPacket(mFileName, mPartition, mNextBlock, block.size(), block.data());

        AnswerQueue::get().sendToClient(packet, adresse);
    }
}
