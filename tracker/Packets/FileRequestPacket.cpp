#include "FileRequestPacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

#include "FileAnswerPacket.hpp"
#include "../AnswerQueue.hpp"
#include "../KnowledgeBase.hpp"


FileRequestPacket::FileRequestPacket(std::string filename, bool send)
                        : mFileName(filename), mSend(send) {
}

FileRequestPacket::FileRequestPacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        delete[] data;
        throw std::runtime_error("Erreur lors du traitement d'un paquet FileRequest\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    bool* send = (bool*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mSend = *send;

}

FileRequestPacket::~FileRequestPacket() {

}

unsigned int FileRequestPacket::getOpcode() {
    return FILE_REQUEST;
}

int FileRequestPacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(bool);
}

char* FileRequestPacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    bool* send = (bool*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    *send = mSend;

    return data;
}

void FileRequestPacket::exec(std::string adresse) {
    Client& c = KnowledgeBase::get().getClient(adresse);
    c.alive();

    std::vector<Association> assocs;
    if (mSend) {
        assocs = KnowledgeBase::get().getClientsToSend(mFileName);
    } else {
        assocs = KnowledgeBase::get().getClientsToAsk(mFileName);
    }
    FileAnswerPacket* fap = new FileAnswerPacket(mFileName, mSend, assocs);
    AnswerQueue::get().sendToClient(fap, adresse);
}
