#include "ClientKnowledgeBase.hpp"

#include <stdexcept>
#include <sstream>
#include <ofstream>
#include <ifstream>
#include <string.h>

#include "defines.hpp"

ClientKnowledgeBase::ClientKnowledgeBase() {
    pthread_mutex_init (&mMutex, NULL);
}

void ClientKnowledgeBase::lock() {
    if (pthread_mutex_lock(&mMutex) != 0) {
        throw std::runtime_error("Erreur lors du lock du mutex de la ClientKnowledgeBase");
    }
}

void ClientKnowledgeBase::unlock() {
    if (pthread_mutex_unlock(&mMutex) != 0) {
        throw std::runtime_error("Erreur lors de l'unlock du mutex de la ClientKnowledgeBase");
    }
}

void ClientKnowledgeBase::lock(std::string filename) {
    pthread_mutex_t mutex = mFilesMutexes[filename];
    if (mutex == (pthread_mutex_t) 0) {
        mFilesMutexes[filename] = PTHREAD_MUTEX_INITIALIZER;
    }
    if (pthread_mutex_lock(&mFilesMutexes[filename]) != 0) {
        throw std::runtime_error("Erreur lors du lock du mutex pour le fichier " + filename);
    }
}

void ClientKnowledgeBase::unlock(std::string filename) {
    pthread_mutex_t mutex = mFilesMutexes[filename];
    if (mutex == (phtread_mutex_t) 0) {
        throw new std::runtime_error("Erreur lors de l'unlock du mutex pour le fichier " + filename + " : mutex inexistant");
    }
    if (pthread_mutex_unlock(&mFilesMutexes) != 0) {
        throw new std::runtime_error("Erreur lors de l'unlock du mutex pour le fichier " + filename);
    }
}

std::vector<int> ClientKnowledgeBase::getPartitions(std::string filename) {
    lock();
    std::vector<int> res = mPartitions[filename];
    unlock();
    return res;
}

std::string ClientKnowledgeBase::blocksMapKey(std::string filename, int partitionNb) {
    std::string key = filename;
    std::ostringstream converter;
    converter << partitionNb;
    key += converter.str();
    return key;
}

std::vector<int> ClientKnowledgeBase::getBlocks(std::string filename, int partitionNb) {
    lock();
    std::vector<int> res = mBlocks[blocksMapKey(filename, partitionNb)];
    unlock();
    return res;
}

bool ClientKnowledgeBase::hasPartition(std::string filename, int partitionNb) {
    lock();
    std::vector<int> partitions = getPartitions(filename);
    for (unsigned int i = 0; i < partitions.size(); ++i) {
        if (partitionNb == partitions.at(i)) {
                unlock();
            return true;
        }
    }
    unlock();
    return false;
}

bool ClientKnowledgeBase::hasBlock(std::string filename, int partitionNb, int block) {
    lock();
    std::vector<int> blocks = getBlocks(filename, partitionNb);
    for (unsigned int i = 0; i < blocks.size(); ++i) {
        if (block == blocks.at(i)) {
            unlock();
            return true;
        }
    }
    unlock();
    return false;
}

void ClientKnowledgeBase::addPartition(std::string filename, int partitionNb) {
    lock();
    if (!hasPartition(filename, partitionNb)) {
        mPartitions[filename].push_back(partitionNb);
        //TODO vider une liste de numéros de block pour une partition complète puisque ça devient inutile ?
    }
    unlock();
}

void ClientKnowledgeBase::addBlock(std::string filename, int partitionNb, int block) {
    lock();
    if (!hasBlock(filename, partitionNb, block)) {
        mBlocks[blocksMapKey(filename, partitionNb)].push_back(block);
    }
    unlock();
}

void ClientKnowledgeBase::getBlockData(std::string filename, int partition, int block, char* buffer, int bufferSize) {
    if (bufferSize < BLOCK_SIZE) {
        throw std::runtime_error("Buffer trop petit pour getBLockData");
    }
    if (!hasBlock(filename, partition, block)) {
        memset(buffer, 0, BLOCK_SIZE);
        return;
    }
    lock(filename);
    std::ifstream file = std::ifstream(FILES_PATH + filename);

    long long offset = partition * block * BLOCK_SIZE;
    file.seekg(0, file.end);
    long long length = file.tellg();
    if (offset > length) {
        throw std::runtime_error("Offset de lecture supérieur à la taille du fichier");
    }
    file.seekg(offset, file.beg);

    file.read(buffer, BLOCK_SIZE);

    file.close();

    unlock(filename);
}

void ClientKnowledgeBase::setBlockData(std::string filename, int partition, int block, char* data, int bufferSize) {
    if (bufferSize < BLOCK_SIZE) {
        throw std::runtime_error("Buffer trop petit pour setBLockData");
    }
    lock(filename);
    std::ofstream file = std::ofstream(FILES_PATH + filename);

    long long offset = partition * block * BLOCK_SIZE;
    file.seekg(0, file.end);
    long long length = file.tellg();
    if (offset > length) {
        throw std::runtime_error("Offset de lecture supérieur à la taille du fichier");
    }
    file.seekg(offset, file.beg);

    file.write(buffer, BLOCK_SIZE);

    file.close();

    addBlock(filename, partition, block);

    unlock(filename);
}

int ClientKnowledgeBase::getNextFreeBlockNumber(std::string filename, int partition) {
    std::vector<int> blocks = mBlocks[blocksMapKey(filename, partition)];
    int b = -1;
    lock();
    for (unsigned int i = 0; i < blocks.size(); ++i) {
        if (b + 1 != blocks.at(i)) {
            unlock();
            return b + 1;
        }
    }
    unlock();
    return -1;
}
