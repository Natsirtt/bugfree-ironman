#include "FileAnswerPacket.hpp"
#include "../Opcode.hpp"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

#include "../ClientKnowledgeBase.hpp"
#include "RRQPacket.hpp"
#include "WRQPacket.hpp"
#include "../AnswerQueue.hpp"
#include "../Defines.hpp"

FileAnswerPacket::FileAnswerPacket(std::string filename, bool send, long long int filesize, std::vector<Association> assoc)
                                : mFileName(filename), mSend(send), mFilesize(filesize), mAssoc(assoc) {
    if (mAssoc.size() > MAX_ASSOC_NUMBER) {
        throw std::runtime_error("Erreur lors de la construction du paquet FileAnswer : trop d'associations\n");
    }
}

FileAnswerPacket::FileAnswerPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        std::cout << size << " " << getSize() << std::endl;
        throw std::runtime_error("Erreur lors du traitement d'un paquet FileAnswer\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    bool* send = (bool*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mSend = *send;

    long long int* fsize = (long long int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool));
    mFilesize = *fsize;

    int* nbAssocPtr = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(long long int));
    int nbAssoc = htonl(*nbAssocPtr);

    Association* assocs = (Association*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(long long int) + sizeof(int));
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
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(long long int) + sizeof(int) + sizeof(Association) * MAX_ASSOC_NUMBER;
}

char* FileAnswerPacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    bool* send = (bool*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    *send = mSend;

    long long int* fsize = (long long int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool));
    *fsize = mFilesize;

    int* nbAssoc = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(long long int));
    *nbAssoc = htonl(mAssoc.size());

    int offset = sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(long long int) + sizeof(int);
    memcpy(data + offset, mAssoc.data(), sizeof(Association) * mAssoc.size());

    return data;
}

void FileAnswerPacket::exec(std::string adresse) {
    std::cout << "exec FileAnswer taille : " << mAssoc.size() << std::endl;

    for (unsigned int i = 0; i < mAssoc.size(); ++i) {
        std::string addr(mAssoc[i].ipClient);
        IPacket* packet = NULL;

        if (mSend) {
            long long int filesize = ClientKnowledgeBase::get().getFile(mFileName).getSize();
            packet = new WRQPacket(mFileName, filesize, mAssoc[i].partition);
        } else {
            int firstBlock = ClientKnowledgeBase::get().getNextFreeBlockNumber(mFileName, mAssoc[i].partition);
            packet = new RRQPacket(mFileName, mAssoc[i].partition, firstBlock);
            // TODO ajouter la taille:: dans la clientKnowledgeBase
        }

        if (addr == "127.0.0.1") {
            addr = State::get().getTrackerIp();
        }

        AnswerQueue::get().sendToClient(packet, addr);
    }
}
