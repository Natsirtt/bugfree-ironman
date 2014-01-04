#include "FileAnswerPacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

#include "../ClientKnowledgeBase.hpp"
#include "RRQPacket.hpp"
#include "WRQPacket.hpp"
#include "../AnswerQueue.hpp"

FileAnswerPacket::FileAnswerPacket(std::string filename, bool send, std::vector<Association> assoc) : mFileName(filename), mSend(send), mAssoc(assoc) {
    if (mAssoc.size() > MAX_ASSOC_NUMBER) {
        throw std::runtime_error("Erreur lors de la construction du paquet FileAnswer : trop d'associations\n");
    }
}

FileAnswerPacket::FileAnswerPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        delete[] data;
        throw std::runtime_error("Erreur lors du traitement d'un paquet FileAnswer\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    bool* send = (bool*) sizeof(getOpcode()) + MAX_FILENAME_SIZE;
    mSend = *send;

    int* nbAssocPtr = (int*) sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool);
    int nbAssoc = htonl(*nbAssocPtr);

    Association* assocs = (Association*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(int));
    for (int i = 0; i < nbAssoc; ++i) {
        mAssoc.push_back(assocs[i]);
    }
}

FileAnswerPacket::~FileAnswerPacket() {

}

unsigned int FileAnswerPacket::getOpcode() {
    return FILE_ANSWER;
}

int FileAnswerPacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(int) + sizeof(Association) * MAX_ASSOC_NUMBER;
}

char* FileAnswerPacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    bool* send = (bool*) sizeof(getOpcode()) + MAX_FILENAME_SIZE;
    *send = mSend;

    int* nbAssoc = (int*) sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool);
    *nbAssoc = htonl(mAssoc.size());

    int offset = sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(int);
    for (unsigned int i = 0; i < mAssoc.size(); ++i) {
        memcpy(data + offset + i * sizeof(Association), &mAssoc[i], sizeof(Association));
    }

    return data;
}

void FileAnswerPacket::exec(std::string adresse) {
    for (unsigned int i = 0; i < mAssoc.size(); ++i) {
        std::string addr(mAssoc[i].ipClient);
        IPacket* packet = NULL;

        if (mSend) {
            packet = new WRQPacket(mFileName, mAssoc[i].partition);
        } else {
            int firstBlock = ClientKnowledgeBase::get().getNextFreeBlockNumber(mFileName, mAssoc[i].partition);
            packet = new RRQPacket(mFileName, mAssoc[i].partition, firstBlock);
        }

        AnswerQueue::get().sendToClient(packet, addr);
    }
}
