#include "ClientFile.hpp"

#include <fstream>
#include <stdexcept>
#include <climits>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sstream>

#include "Defines.hpp"
#include "Packets/FileRequestPacket.hpp"
#include "Packets/FileUpdatePacket.hpp"
#include "AnswerQueue.hpp"

ClientFile::ClientFile() : mFilename(""), mFileSize(0) {
    std::cout << "Nouveau fichier vide" << std::endl;
}

bool ClientFile::isCorrectFile() {
    return mFilename != "";
}

ClientFile::ClientFile(std::string filename, long long int fileSize, bool init) : mFilename(filename), mFileSize(fileSize)
{
    std::cout << "ClientFile(std::string filename, long long int fileSize)" << std::endl;
    if (pthread_mutex_init(&mMutex, NULL) != 0)
    {
        perror("Erreur init mutex");
        throw std::runtime_error("Erreur init mutex dans ClientFile");
    }
    std::cout << fileSize << std::endl;
    long long int part_size = PARTITION_SIZE;
    long long int nbOfPart = fileSize / part_size;
    if (fileSize % part_size != 0) {
        nbOfPart++;
    }
    int bitmapSize = nbOfPart / 8;
    if ((nbOfPart % 8) != 0) {
        bitmapSize++;
    }
    mPartitions.resize(bitmapSize);
    mPartitionsInProgress.resize(bitmapSize);
    if (init) {
        for (long long i = 0; i < nbOfPart; ++i) {
            //std::cout << i << std::endl;
            endPartition(i);
        }
    }

}

ClientFile::ClientFile(std::string filename)
{
    std::cout << "ClientFile(std::string filename)" << std::endl;
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

bool ClientFile::isNthBitSet(std::vector<char>& bitmap, int n) {
    if (n < (int)(bitmap.size() * 8)) {
        return isNthBitSet(bitmap.at(n / 8), n % 8);
    }
    return false;
}

void ClientFile::setNthBit(std::vector<char>& bitmap, int n) {
    char c = bitmap.at(n / 8);
    static unsigned char mask[] = {128, 64, 32, 16, 8, 4, 2, 1};
    bitmap[n / 8] = c | mask[n % 8];
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
    return hasPartition(part) || isPartitionInProgress(part);
}

void ClientFile::addBlock(int part, int block) {
    setNthBit(mBlocks[part], block);

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
    if (!isPartitionInProgress(part)) {
        setNthBit(mPartitionsInProgress, part);
        mBlocks[part].resize(BLOCK_PER_PARTITION);
    }
}

void ClientFile::endPartition(int part) {
    //if (!hasPartition(part)) {
        setNthBit(mPartitions, part);
    //}
}

std::vector<char> ClientFile::getBlockData(int part, int block) {
    if (!hasPartition(part)) {
        std::cout << "pas de partition" << std::endl;
        return std::vector<char>();
    }
    lock();
    std::fstream file((std::string(FILES_PATH) + mFilename).c_str());

    long long offset = computeFileOffset(part, block);

    if (offset > mFileSize) {
        throw std::runtime_error("Offset de lecture supérieur à la taille du fichier");
    }
    file.seekg(offset);

    char buffer[BLOCK_SIZE];
    file.read(buffer, BLOCK_SIZE);
    int n = file.gcount();

    file.close();

    unlock();

    return std::vector<char>(buffer, buffer + n);
}

void ClientFile::setBlockData(int part, int block, std::vector<char>& data) {
    std::cout << "postLock" << std::endl;
    lock();
    std::cout << "estLock" << std::endl;
    if (hasBlock(part, block)) {
        unlock();
        return;
    }
    std::cout << "hasBlock" << std::endl;

    std::string absoluteFileName = std::string(FILES_PATH) + mFilename;

    if (!isPartitionAcquiredOrInProgress(part)) {
        std::cout << "non acquis " << mPartitions.size() << std::endl;
        int firstPart = getFirstUsedBit(mPartitions);
        std::cout << "getFirst " << firstPart << std::endl;
        int lastPart = getLastUsedBit(mPartitions);
        std::cout << "getLast " << lastPart << std::endl;
        if (firstPart == -1) {
            createFile(absoluteFileName, std::min(PARTITION_SIZE, mFileSize));
        } else {
            if ((part < firstPart) || (part > lastPart)) {
                std::fstream file((std::string(FILES_PATH) + mFilename).c_str(), std::fstream::binary | std::fstream::in);
                //file.seekg(0, file.end);
                //long long currentSize = file.tellg();
                //file.seekg(0, file.beg);

                if (firstPart > part) {
                    long long gap = (firstPart - part) * PARTITION_SIZE;
                    long long newSize = (firstPart - part + 1) * PARTITION_SIZE;
                    std::string newFilePath = std::string(FILES_PATH) + mFilename + ".tmp";
                    createFile(newFilePath, newSize);

                    std::fstream newFile(newFilePath.c_str(), std::fstream::binary | std::fstream::out);
                    newFile.seekp(gap, newFile.beg);

                    char buffer[1024 * 1024];
                    while (!file.eof()) {
                        file.read(buffer, 1024 * 1024);
                        int n = file.gcount();
                        newFile.write(buffer, n);
                    }
                    file.close();
                    newFile.close();
                    if (remove(absoluteFileName.c_str()) == -1) {
                        perror("Error of remove");
                        throw std::runtime_error("Error of remove");
                    }
                    if (rename(newFilePath.c_str(), absoluteFileName.c_str()) == -1) {
                        perror("Error of rename");
                        throw std::runtime_error("Error of rename");
                    }
                } else {
                    long long newSize = (part - firstPart + 1) * PARTITION_SIZE;
                    std::cout << (std::string(FILES_PATH) + mFilename).c_str() << " " << PARTITION_SIZE << std::endl;
                    if (truncate(absoluteFileName.c_str(), newSize) == -1) {
                        perror("Erreur au truncate3");
                        throw std::runtime_error("Erreur de truncate3");
                    }
                }
            }
        }
        std::cout << "preBegin" << std::endl;
        beginPartition(part);
        std::cout << "postBegin" << std::endl;
    }
    std::cout << "openFile" << std::endl;
    std::fstream file(absoluteFileName.c_str());
    std::cout << absoluteFileName << " estOpen " << file.good() << std::endl;

    long long offset = computeFileOffset(part, block);

    if (offset > mFileSize) {
        throw std::runtime_error("Offset de lecture supérieur à la taille du fichier");
    }
    file.seekp(offset);
    file.write(data.data(), data.size());
    file.close();
    addBlock(part, block);

    unlock();
}

int ClientFile::getFirstFreeBit(std::vector<char>& bitmap) {
    for (unsigned int i = 0; i < (bitmap.size() * 8); ++i) {
        if (!isNthBitSet(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

int ClientFile::getFirstUsedBit(std::vector<char>& bitmap) {
    for (unsigned int i = 0; i < (bitmap.size() * 8); ++i) {
        if (isNthBitSet(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

int ClientFile::getLastFreeBit(std::vector<char>& bitmap) {
    for (int i = (bitmap.size() * 8) - 1; i >= 0; --i) {
        if (!isNthBitSet(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

int ClientFile::getLastUsedBit(std::vector<char>& bitmap) {
    for (int i = ((bitmap.size() * 8) - 1); i >= 0; --i) {
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
    int firstPartition = getFirstFreeBit(mPartitionsInProgress);
    int tmp = getFirstFreeBit(mPartitions);
    if ((tmp < firstPartition) && (tmp != -1)) {
        firstPartition = tmp;
    }

    if (firstPartition == -1) {
        return 0 + block * BLOCK_SIZE;
    }

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
    IPacket* packet = new FileUpdatePacket(mFilename, mPartitions.size(), mPartitions.data());
    AnswerQueue::get().sendToTracker(packet, trackerIP);
    packet = new FileRequestPacket(mFilename, true, mFileSize);
    AnswerQueue::get().sendToTracker(packet, trackerIP);
    unlock();
}

void ClientFile::createFile(std::string& filename, long long int size) {
    std::fstream newFile(filename.c_str(), std::fstream::binary | std::fstream::out | std::fstream::trunc);
    newFile << " ";
    newFile.close();
    std::cout << "create file " << filename << " " << size << std::endl;;
    if (truncate(filename.c_str(), size) == -1) {
        perror("Erreur au truncate1");
        throw std::runtime_error("Erreur de truncate1");
    }
}

