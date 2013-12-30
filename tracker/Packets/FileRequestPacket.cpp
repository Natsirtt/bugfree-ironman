#include "FileRequestPacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

#include "FileAnswerPacket.hpp"

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
    mSend = ntohl(*send);

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

    int* send = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    *send = htonl(mSend);

    return data;
}

void FileRequestPacket::exec(std::string adresse) {
    std::vector<Association> assocs;

    // TODO Recupérer les associations

    FileAnswerPacket* fap = new FileAnswerPacket(mFileName, assocs);

    // TODO envoyer le paquet
}
