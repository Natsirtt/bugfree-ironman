#include "DataPacket.hpp"
#include "../Opcode.hpp"

#include "../AnswerQueue.hpp"
#include "../ClientKnowledgeBase.hpp"
#include "ACKPacket.hpp"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <fstream>

DataPacket::DataPacket(std::string filename, int partition, int blockNb, int blockSize, char* blockData)
                        : mFileName(filename), mPartition(partition), mBlockNb(blockNb), mBlockSize(blockSize), mBlockData(blockData) {
    if (blockSize > MAX_DATA_SIZE) {
        throw std::runtime_error("Erreur lors de la creation d'un paquet DATA : Taille des donnees trop importante\n");
    }
}

DataPacket::DataPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        throw std::runtime_error("Erreur lors du traitement d'un paquet DATA\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mPartition = ntohl(*partNb);

    int* blockNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int));
    mBlockNb = htonl(*blockNb);

    int* blockSize = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int) + sizeof(int));
    mBlockSize = htonl(*blockSize);

    char* blockData = (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int) + sizeof(int) + sizeof(int));

    int maxSize = MAX_DATA_SIZE;
    maxSize = std::min(maxSize, mBlockSize);
    mBlockData = new char[maxSize];

    memcpy(mBlockData, blockData, maxSize);
    std::stringstream ss;
    ss << mBlockNb;
    std::fstream file2(ss.str().c_str(), std::fstream::out | std::fstream::trunc);
    file2.write(mBlockData, mBlockSize);
    file2.close();

}

DataPacket::~DataPacket() {

}

unsigned int DataPacket::getOpcode() {
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

    int* blockNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int));
    *blockNb = htonl(mBlockNb);

    int* blockSize = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int) + sizeof(int));
    *blockSize = htonl(mBlockSize);

    char* blockData = (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int) + sizeof(int) + sizeof(int));
    int maxSize = MAX_DATA_SIZE;
    memcpy(blockData, mBlockData, std::min(mBlockSize, maxSize));
    std::stringstream ss;
    ss << mBlockNb;
    std::fstream file2(ss.str().c_str(), std::fstream::out | std::fstream::trunc);
    file2.write(mBlockData, mBlockSize);
    file2.close();
    return data;
}

void DataPacket::exec(std::string adresse) {
    std::cout << "exec DataPacket" << std::endl;
    std::vector<char> data(mBlockData, mBlockData + mBlockSize);
    ClientKnowledgeBase::get().setBlockData(mFileName, mPartition, mBlockNb, data);
    std::cout << "finSetBlock" << std::endl;
    int next = ClientKnowledgeBase::get().getNextFreeBlockNumber(mFileName, mPartition);
    std::cout << "prochain block " << next << std::endl;
    if (next > 0) {
        IPacket* packet = new ACKPacket(mFileName, mPartition, mBlockNb, next);
        AnswerQueue::get().sendToClient(packet, adresse);
    }
}

std::string DataPacket::getName() {
    return mFileName;
}

int DataPacket::getPartitionNb() {
    return mPartition;
}

int DataPacket::getBlockNb() {
    return mBlockNb;
}

int DataPacket::getBlockSize() {
    return mBlockSize;
}

void DataPacket::getData(char *buffer, int bufferSize) {
    if (bufferSize < mBlockSize) {
        throw std::runtime_error("Erreur lors de la récupération des données d'un paquet DATA : buffer trop petit\n");
    }
    strncpy(buffer, mBlockData, bufferSize);
    //return 0;
}
