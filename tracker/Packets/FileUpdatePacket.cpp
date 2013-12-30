#include "FileUpdatePacket.hpp"
#include "../Opcode.hpp"

#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

FileUpdatePacket::FileUpdatePacket(std::string filename, int bitmapSize, char *partitionBitmap)
                        : mFileName(filename), mBitmapSize(bitmapSize), mPartitionBitmap(partitionBitmap) {
}

FileUpdatePacket::FileUpdatePacket(char* data, int size) {
    int* opcode = (int*) data;
    if ((ntohl(*opcode) != getOpcode()) || (size != getSize())) {
        delete[] data;
        throw std::runtime_error("Erreur lors du traitement d'un paquet FILEUPDATE\n");
    }

    char* filename = data + sizeof(getOpcode());
    mFileName = std::string(filename);

    int *bitmapSize = (int *) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE);
    mBitmapSize = *bitmapSize;

    mPartitionBitmap = (char*) (data + sizeof(getOpcode()) + MAX_FILENAME_SIZE + sizeof(int));
}

FileUpdatePacket::~FileUpdatePacket() {

}

unsigned int FileUpdatePacket::getOpcode() {
    return FILE_UPDATE;
}

int FileUpdatePacket::getSize() {
    return sizeof(int) + MAX_FILENAME_SIZE + MAX_BITMAP_SIZE;
}

char* FileUpdatePacket::toData() {
    char* data = new char[getSize()];

    int* opcode = (int*) data;
    *opcode = htonl(getOpcode());

    char* filename = data + sizeof(getOpcode());
    strncpy(filename, mFileName.c_str(), MAX_FILENAME_SIZE - 1);
    filename[MAX_FILENAME_SIZE - 1] = '\0'; // Protection

    int* bitmapSize = data + sizeof(getOpcode()) + MAX_FILENAME_SIZE;
    *bitmapSize = htonl(mBitmapSize);

    char* bitmap = (data + sizeof(getOpcode()) + sizeof(int) + MAX_FILENAME_SIZE);
    memcpy(bitmap, mPartitionBitmap, MAX_BITMAP_SIZE);

    return data;
}

void FileUpdatePacket::exec(std::string adresse) {
    // TODO
}
