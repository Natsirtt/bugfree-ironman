#include "ClientFile.hpp"

#include <fstream>
#include <stdexcept>
#include <climits>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>

#include "Defines.hpp"
#include "Packets/FileRequestPacket.hpp"
#include "Packets/FileUpdatePacket.hpp"
#include "AnswerQueue.hpp"

ClientFile::ClientFile() : mFilename(""), mFileSize(0) {

}

bool ClientFile::isCorrectFile() {
    return mFilename != "";
}

ClientFile::ClientFile(std::string filename, long long fileSize) : mFilename(filename), mFileSize(fileSize)
{
    if (pthread_mutex_init(&mMutex, NULL) != 0)
    {
        perror("Erreur init mutex");
        throw std::runtime_error("Erreur init mutex dans ClientFile");
    }
    long long nbOfPart = fileSize / PARTITION_SIZE;
    if (fileSize % PARTITION_SIZE != 0) {
        nbOfPart++;
    }
    int bitmapSize = nbOfPart / 8;
    if ((nbOfPart % 8) != 0) {
        bitmapSize++;
    }
    mPartitions.resize(bitmapSize);
    for (long long i = 0; i < nbOfPart; ++i) {
        //std::cout << i << std::endl;
        endPartition(i);
    }
}

ClientFile::ClientFile(std::string filename)
{

    if (pthread_mutex_init(&mMutex, NULL) != 0)
    {
        perror("Erreur init mutex");
        throw std::runtime_error("Erreur init mutex dans ClientFile");
    }

    lock();

    std::fstream file((std::string(FILES_PATH) + filename).c_str(), std::fstream::binary | std::fstream::in);
    if (!file.good()) {
        std::cerr << "FILE NOT GOOD !!" << std::endl;
        throw std::runtime_error("File not good after open");
    }
    filename.erase(0, 1);
    mFilename = filename;

    file.seekg(0, file.end);
    long long fileLength = file.tellg();
    file.seekg(0, file.beg);

    char data[fileLength];

    file.read(data, fileLength);
    file.close();

    long long *fileSize = (long long *) data;
    mFileSize = *fileSize;

    unsigned int *bitmapSizePtr = (unsigned int *) (fileSize + 1);
    unsigned int bitmapSize = *bitmapSizePtr;

    char *dataPtr = (char *) (bitmapSizePtr + 1);
    for (unsigned int i = 0; i < bitmapSize; ++i)
    {
        mPartitions.push_back(*(dataPtr + i));
    }

    bitmapSizePtr = (unsigned int *) (dataPtr + bitmapSize);
    bitmapSize = *bitmapSizePtr;
    dataPtr = (char *) (bitmapSizePtr + 1);
    for (unsigned int i = 0; i < bitmapSize; ++i)
    {
        mPartitionsInProgress.push_back(*(dataPtr + i));
    }

    unsigned int *mapSizePtr = (unsigned int *) (dataPtr + bitmapSize);
    unsigned int mapSize = *mapSizePtr;
    int *mapKeyPtr = (int *) (mapSizePtr + 1);
    for (unsigned int i = 0; i < mapSize; ++i)
    {
        int mapKey = *mapKeyPtr;
        bitmapSizePtr = (unsigned int *) (mapKeyPtr + 1);
        bitmapSize = *bitmapSizePtr;
        dataPtr = (char *) (bitmapSizePtr + 1);
        std::vector<char> bitmap;
        for (unsigned int j = 0; j < bitmapSize; ++j)
        {
            bitmap.push_back(*(dataPtr + j));
        }
        mBlocks[mapKey] = bitmap;
        mapKeyPtr = (int *) (dataPtr + bitmapSize);
    }
    unlock();
}

std::string ClientFile::getName() {
    return mFilename;
}

void ClientFile::serialize() {
    lock();
    //Calcul du de la taille du buffer
    long long bufferSize = 0;

    bufferSize += sizeof(long long) + sizeof(unsigned int) + mPartitions.size() * sizeof(char) + sizeof(unsigned int) + mPartitionsInProgress.size() + sizeof(unsigned int);
    for (std::map<int, std::vector<char> >::iterator it = mBlocks.begin(); it != mBlocks.end(); ++it)
    {
        bufferSize += sizeof(unsigned int) + it->second.size();
    }

    char buffer[bufferSize];
    long long *fileLen = (long long *) buffer;
    *fileLen = mFileSize;

    unsigned int *bitmapSize = (unsigned int *) (fileLen + 1);
    *bitmapSize = mPartitions.size();

    char *data = (char *) (bitmapSize + 1);
    for (unsigned int i = 0; i < mPartitions.size(); ++i)
    {
        *(data + i) = mPartitions[i];
    }

    bitmapSize = (unsigned int *) (data + *bitmapSize);
    *bitmapSize = mPartitionsInProgress.size();

    data = (char *) (bitmapSize + 1);
    for (unsigned int i = 0; i < mPartitionsInProgress.size(); ++i)
    {
        *(data + i) = mPartitionsInProgress[i];
    }

    unsigned int *mapSize = (unsigned int *) (data + *bitmapSize);
    *mapSize = mBlocks.size();

    char *ptr = (char *) (mapSize + 1);
    for (std::map<int, std::vector<char> >::iterator it = mBlocks.begin(); it != mBlocks.end(); ++it)
    {
        int *key = (int *) ptr;
        *key = it->first;
        ptr = (char *) (key + 1);
        bitmapSize = (unsigned int *) (ptr);
        *bitmapSize = it->second.size();
        ptr = (char *) (bitmapSize + 1);
        data = ptr;
        for (unsigned int j = 0; j < it->second.size(); ++j)
        {
            *(data + j) = it->second.at(j);
        }
        ptr = data + it->second.size();
    }
    unlock();

    std::string path = std::string(FILES_PATH) + std::string(".") + mFilename;
    std::fstream file(path.c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);
    /*if (truncate(path.c_str(), 0) != 0) {
        perror("truncate");
        throw std::runtime_error("Erreur lors du truncate du fichier metadata");
    }*/

    file.write(buffer, bufferSize);
    file.close();
    if (chmod(path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1) {
        perror("chmod");
        throw std::runtime_error("Erreur lors du chmod sur la metadata");
    }
}

void ClientFile::lock() {
    if (pthread_mutex_lock(&mMutex) != 0)
    {
        perror("erreur lock mutex d'un ClientFile");
        throw std::runtime_error("Erreur lock mutex ClientFile");
    }
}

void ClientFile::unlock() {
    if (pthread_mutex_unlock(&mMutex) != 0)
    {
        perror("erreur unlock mutex d'un ClientFile");
        throw std::runtime_error("Erreur lock mutex ClientFile");
    }
}

bool ClientFile::isNthBitSet(char c, int n) {
    static unsigned char mask[] = {128, 64, 32, 16, 8, 4, 2, 1};
    return ((c & mask[n]) != 0);
}

bool ClientFile::isNthBitSet(std::vector<char> bitmap, int n) {
    return (bitmap.size() >= (unsigned int) (n / 8)) && isNthBitSet(bitmap[n / 8], n % 8);
}

void ClientFile::setNthBit(std::vector<char> &bitmap, int n) {
    char c = bitmap[n / 8];
    char mask = (char) 1; //00000001
    mask = mask << (n % 8);
    bitmap[n / 8] = c | mask;
}

bool ClientFile::hasPartition(int part) {
    return isNthBitSet(mPartitions, part);
}

bool ClientFile::hasBlock(int part, int block) {
    return isNthBitSet(mBlocks[part], block);
}

bool ClientFile::isPartitionInProgress(int part) {
    if (hasPartition(part)) {
        return false;
    }
    return isNthBitSet(mPartitionsInProgress, part);
}

bool ClientFile::isPartitionAcquiredOrInProgress(int part) {
    return hasPartition(part) || isPartitionAcquiredOrInProgress(part);
}

void ClientFile::addBlock(int part, int block) {
    if (!hasBlock(part, block)) {
        setNthBit(mBlocks[part], block);
    }
    bool partCompleted = true;
    for (unsigned int i = 0; i < PARTITION_SIZE / BLOCK_SIZE; ++i) {
        if (!hasBlock(part, i)) {
            partCompleted = false;
            break;
        }
    }
    if (partCompleted) {
        endPartition(part);
    }
}

void ClientFile::beginPartition(int part) {
    lock();
    if (!isPartitionInProgress(part)) {
        setNthBit(mPartitionsInProgress, part);
    }
    unlock();
}

void ClientFile::endPartition(int part) {
    lock();
    //if (!hasPartition(part)) {
        setNthBit(mPartitions, part);
    //}
    unlock();
}

std::vector<char> ClientFile::getBlockData(int part, int block) {
    if (!hasBlock(part, block)) {
        return std::vector<char>();
    }
    lock();
    std::fstream file((std::string(FILES_PATH) + mFilename).c_str(), std::fstream::binary | std::fstream::in);

    long long offset = computeFileOffset(part, block);

    if (offset > mFileSize) {
        throw std::runtime_error("Offset de lecture supérieur à la taille du fichier");
    }
    file.seekg(offset, file.beg);

    char buffer[BLOCK_SIZE];
    file.read(buffer, BLOCK_SIZE);

    file.close();

    unlock();

    return std::vector<char>(buffer, buffer + BLOCK_SIZE);
}

void ClientFile::setBlockData(int part, int block, std::vector<char> data) {
    lock();
    if (hasBlock(part, block)) {
        return;
    }

    if (!isPartitionAcquiredOrInProgress(part)) {
        int firstPart = getFirstUsedBit(mPartitions);
        int lastPart = getLastUsedBit(mPartitions);
        if (!((firstPart < part) && (part < lastPart))) {
            std::fstream file((std::string(FILES_PATH) + mFilename).c_str(), std::fstream::binary | std::fstream::in);
            file.seekg(0, file.end);
            long long currentSize = file.tellg();
            file.seekg(0, file.beg);

            long long gap = 0;
            if (firstPart > part) {
                gap = (firstPart - part) * BLOCK_SIZE;
                std::string newFilePath = std::string(FILES_PATH) + mFilename + ".tmp";
                std::fstream newFile(newFilePath.c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);
                /*if (truncate(newFilePath.c_str(), currentSize + gap) == -1) {
                    perror("Erreur au truncate");
                    throw std::runtime_error("Erreur de truncate");
                }*/

                newFile.seekp(gap, newFile.beg);

                char buffer[1024 * 1024];
                while (!file.eof()) {
                    file.read(buffer, 1024 * 1024);
                    newFile.write(buffer, 1024 * 1024);
                }
                file.close();
                newFile.close();
                if (remove((std::string(FILES_PATH) + mFilename).c_str()) == -1) {
                    perror("Error of remove");
                    throw std::runtime_error("Error of remove");
                }
                if (rename(newFilePath.c_str(), (std::string(FILES_PATH) + mFilename).c_str()) == -1) {
                    perror("Error of rename");
                    throw std::runtime_error("Error of rename");
                }

            } else {
                gap = (part - lastPart) * BLOCK_SIZE;
                if (truncate((std::string(FILES_PATH) + mFilename).c_str(), currentSize + gap) == -1) {
                    perror("Erreur au truncate");
                    throw std::runtime_error("Erreur de truncate");
                }
            }
        }
        beginPartition(part);
    }

    std::fstream file((std::string(FILES_PATH) + mFilename).c_str(), std::fstream::binary | std::fstream::out);

    long long offset = computeFileOffset(part, block);

    if (offset > mFileSize) {
        throw std::runtime_error("Offset de lecture supérieur à la taille du fichier");
    }
    file.seekp(offset, file.beg);

    file.write(data.data(), data.size());

    file.close();

    addBlock(part, block);

    unlock();
}

int ClientFile::getFirstFreeBit(std::vector<char> bitmap) {
    for (unsigned int i = 0; i < bitmap.size() * sizeof(char); ++i) {
        if (!isNthBitSet(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

int ClientFile::getFirstUsedBit(std::vector<char> bitmap) {
    for (unsigned int i = 0; i < bitmap.size() * sizeof(char); ++i) {
        if (isNthBitSet(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

int ClientFile::getLastFreeBit(std::vector<char> bitmap) {
    for (unsigned int i = bitmap.size() * sizeof(char); i > 0; ++i) {
        if (!isNthBitSet(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

int ClientFile::getLastUsedBit(std::vector<char> bitmap) {
    for (unsigned int i = bitmap.size() * sizeof(char); i > 0; ++i) {
        if (isNthBitSet(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

int ClientFile::getNextFreeBlockNumber(int part) {
    lock();
    int res = getFirstFreeBit(mBlocks[part]);
    unlock();
    return res;
}

long long ClientFile::computeFileOffset(int partitionNb, int block) {
    int firstPartition = INT_MAX;

    lock();

    for (unsigned int i = 0; i < mPartitionsInProgress.size(); ++i) {
        if (mPartitionsInProgress.at(i) < firstPartition) {
            firstPartition = mPartitionsInProgress.at(i);
        }
    }
    unlock();
    return (partitionNb - firstPartition) * PARTITION_SIZE + block * BLOCK_SIZE;
}

long long ClientFile::getSize() {
    lock();
    long long res = mFileSize;
    unlock();
    return res;
}

void ClientFile::send(std::string& trackerIP) {
    lock();
    /*IPacket* packet = new FileUpdatePacket(mFilename, mPartitions.size(), mPartitions.data());
    AnswerQueue::get().sendToTracker(packet, trackerIP);*/

    IPacket* packet = new FileRequestPacket(mFilename, true, mFileSize);
    AnswerQueue::get().sendToTracker(packet, trackerIP);
    unlock();
}
