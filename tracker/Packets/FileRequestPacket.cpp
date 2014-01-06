#include "FileRequestPacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

#include "FileAnswerPacket.hpp"
#include "../AnswerQueue.hpp"
#include "../KnowledgeBase.hpp"
#include "../Defines.hpp"


FileRequestPacket::FileRequestPacket(std::string filename, bool send, long long filesize)
                        : mFileName(filename), mSend(send), mFilesize(filesize) {
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

    long long int* fsize = (long long int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(bool));
    mFilesize = *fsize;
}

FileRequestPacket::~FileRequestPacket() {

}

unsigned int FileRequestPacket::getOpcode() {
    return FILE_REQUEST;
}

int FileRequestPacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(bool) + sizeof(long long int);
}

char* FileRequestPacket::toData() {
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

    return data;
}

void FileRequestPacket::exec(std::string adresse) {
    Client& c = KnowledgeBase::get().getClient(adresse);
    c.alive();

    std::vector<Association> assocs;
    int fileSize = 0;
    if (mSend) {
        File f(mFileName, mFilesize, mFilesize / PARTITION_SIZE);
        KnowledgeBase::get().addFile(f);
        assocs = KnowledgeBase::get().getClientsToSend(mFileName);
    } else {
        File& f = KnowledgeBase::get().getFile(mFileName);
        fileSize = f.getSize();
        assocs = f.getClientsToAsk();
    }
    FileAnswerPacket* fap = new FileAnswerPacket(mFileName, mSend, fileSize, assocs);
    AnswerQueue::get().sendToClient(fap, adresse);
}
