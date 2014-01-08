#include "WRQPacket.hpp"
#include "../Opcode.hpp"
#include "../ClientKnowledgeBase.hpp"
#include "../AnswerQueue.hpp"
#include "ACKPacket.hpp"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

WRQPacket::WRQPacket(std::string filename, long long int filesize, int partition) : mFileName(filename), mFileSize(filesize), mPartition(partition) {
}

WRQPacket::WRQPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        throw std::runtime_error("Erreur lors du traitement d'un paquet RRQ\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    long long int* filesize = (long long int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mFileSize = *filesize; // Pas de ntoh pour les long long int

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(long long int));
    mPartition = ntohl(*partNb);

}

WRQPacket::~WRQPacket() {

}

unsigned int WRQPacket::getOpcode() {
    return WRQ;
}

int WRQPacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(long long int) + sizeof(int);
}

char* WRQPacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    long long int* filesize = (long long int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    *filesize = mFileSize; // Pas de ntoh pour les long long int

    int* partNb = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(long long int));
    *partNb = htonl(mPartition);

    return data;
}

void WRQPacket::exec(std::string adresse) {
    std::cout << "exec WRQPacket" << std::endl;

    ClientKnowledgeBase::get().addClientFile(ClientFile(mFileName, mFileSize, false));
    IPacket* packet = new ACKPacket(mFileName, mPartition, -1, 0);

    AnswerQueue::get().sendToClient(packet, adresse);
}

std::string WRQPacket::getName() {
    return mFileName;
}

int WRQPacket::getPartitionNb() {
    return mPartition;
}
