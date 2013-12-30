#include "FileAnswerPacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

FileAnswerPacket::FileAnswerPacket(std::string filename, std::vector<Association> assoc) : mFileName(filename), mAssoc(assoc) {
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

    int* nbAssocPtr = (int*) sizeof(getOpcode()) + MAX_FILENAME_SIZE;
    int nbAssoc = htonl(*nbAssocPtr);

    Association* assocs = (Association*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int));
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
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(int) + sizeof(Association) * MAX_ASSOC_NUMBER;
}

char* FileAnswerPacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    int* nbAssoc = (int*) sizeof(getOpcode()) + MAX_FILENAME_SIZE;
    *nbAssoc = htonl(mAssoc.size());

    int offset = sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int);
    for (unsigned int i = 0; i < mAssoc.size(); ++i) {
        memcpy(data + offset + i * sizeof(Association), &mAssoc[i], sizeof(Association));
    }

    return data;
}

void FileAnswerPacket::exec(std::string adresse) {
    // TODO
}
