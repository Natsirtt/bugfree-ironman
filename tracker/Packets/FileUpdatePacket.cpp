#include "FileUpdatePacket.hpp"
#include "../Opcode.hpp"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

#include "../KnowledgeBase.hpp"

FileUpdatePacket::FileUpdatePacket(std::string filename, int bitmapSize, char *partitionBitmap)
                        : mFileName(filename), mBitmapSize(bitmapSize), mPartitionBitmap(partitionBitmap) {
    if (mBitmapSize > MAX_BITMAP_SIZE) {
        throw std::runtime_error("Erreur lors du traitement d'un paquet FILEUPDATE : taille de la bitmap trop eleve\n");
    }
}

FileUpdatePacket::FileUpdatePacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        throw std::runtime_error("Erreur lors du traitement d'un paquet FILEUPDATE\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    int* bitmapSize = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mBitmapSize = htonl(*bitmapSize);

    char* partitionBitmap = (char*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int));
    mPartitionBitmap = new char[mBitmapSize];
    memcpy(mPartitionBitmap, partitionBitmap, mBitmapSize);
}

FileUpdatePacket::~FileUpdatePacket() {
}

unsigned int FileUpdatePacket::getOpcode() {
    return FILE_UPDATE;
}

int FileUpdatePacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + sizeof(int) + MAX_BITMAP_SIZE;
}

char* FileUpdatePacket::toData() {

    std::cout << "FileUpdate" << std::endl;
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    int* bitmapSize = (int*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    *bitmapSize = htonl(mBitmapSize);

    char* bitmap = (data + sizeof(getOpcode()) + sizeof(int) + MAX_FILENAME_SIZE);

    int maxSize = MAX_BITMAP_SIZE;
    maxSize = std::min(maxSize, mBitmapSize);
    memcpy(bitmap, mPartitionBitmap, maxSize);

    return data;
}

void FileUpdatePacket::exec(std::string adresse) {
    Client& c = KnowledgeBase::get().getClient(adresse);
    c.alive();
    c.updateFile(mFileName, mPartitionBitmap, mBitmapSize);

    File& f = KnowledgeBase::get().getFile(mFileName);
    for (int i = 0; i < f.getPartitionsNb(); ++i) {
        if (c.hasPartition(mFileName, i)) {
            f.addClient(&c, i);
        }
    }
}
