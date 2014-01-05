
#include "ClientKnowledgeBase.hpp"

#include <climits>
#include <stdexcept>
#include <sstream>
#include <fstream>
//#include <ofstream>
//#include <ifstream>
#include <string.h>

#include "Defines.hpp"

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
    lock();
    pthread_mutex_t mutex = mFilesMutexes[filename];
    /*if (mutex == (pthread_mutex_t) 0) {
        mFilesMutexes[filename] = PTHREAD_MUTEX_INITIALIZER;
    }*/
    if (pthread_mutex_lock(&mFilesMutexes[filename]) != 0) {
        throw std::runtime_error("Erreur lors du lock du mutex pour le fichier " + filename);
    }
    unlock();
}

void ClientKnowledgeBase::unlock(std::string filename) {
    lock();
    pthread_mutex_t mutex = mFilesMutexes[filename];
    /*if (mutex == (phtread_mutex_t) 0) {
        throw new std::runtime_error("Erreur lors de l'unlock du mutex pour le fichier " + filename + " : mutex inexistant");
    }
    if (pthread_mutex_unlock(&mFilesMutexes) != 0) {
        throw new std::runtime_error("Erreur lors de l'unlock du mutex pour le fichier " + filename);
    }*/
    unlock();
}

std::vector<int> ClientKnowledgeBase::getPartitionsInProgress(std::string filename) {
    lock();
    std::vector<int> res = mPartitionsInProgress[filename];
    unlock();
    return res;
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

bool ClientKnowledgeBase::isGettingPartition(std::string filename, int partitionNb) {
    if (hasPartition(filename, partitionNb)) {
        return false;
    }
    lock();
    std::vector<int> parts = getPartitionsInProgress(filename);
    for (unsigned int i = 0; i < parts.size(); ++i) {
        if (partitionNb == parts.at(i)) {
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

void ClientKnowledgeBase::beginPartition(std::string filename, int partitionNb) {
    lock();
    if (!isGettingPartition(filename, partitionNb)) {
        mPartitionsInProgress[filename].push_back(partitionNb);
    }
    unlock();
}

std::vector<char> ClientKnowledgeBase::getBlockData(std::string filename, int partition, int block) {
    // TODO
    int bufferSize;
    char* buffer;
    if (bufferSize < BLOCK_SIZE) {
        throw std::runtime_error("Buffer trop petit pour getBLockData");
    }
    if (!hasBlock(filename, partition, block)) {
        memset(buffer, 0, BLOCK_SIZE);
        return std::vector<char>();
    }
    lock(filename);
    std::ifstream file((std::string(FILES_PATH) + filename).c_str(), std::ifstream::binary);

    long long offset = computeFileOffset(filename, partition, block, true);
    file.seekg(0, file.end);
    long long length = file.tellg();
    if (offset > length) {
        throw std::runtime_error("Offset de lecture supérieur à la taille du fichier");
    }
    file.seekg(offset, file.beg);

    file.read(buffer, BLOCK_SIZE);

    file.close();

    unlock(filename); // TODO

    return std::vector<char>();
}

void ClientKnowledgeBase::setBlockData(std::string filename, int partition, int block, std::vector<char> data) {
    lock(filename);
    std::fstream file((std::string(FILES_PATH) + filename).c_str());

    long long offset = computeFileOffset(filename, partition, block, false);
    file.seekp(0, file.end);
    long long length = file.tellp();
    if (offset > length) {
        throw std::runtime_error("Offset de lecture supérieur à la taille du fichier");
    }
    file.seekp(offset, file.beg);

    file.write(data.data(), data.size());

    file.close();

    addBlock(filename, partition, block);

    unlock(filename);
}

int ClientKnowledgeBase::getNextFreeBlockNumber(std::string filename, int partition) {
    lock();
    std::vector<int> blocks = mBlocks[blocksMapKey(filename, partition)];
    int b = -1;
    for (unsigned int i = 0; i < blocks.size(); ++i) {
        if (b + 1 != blocks.at(i)) {
            unlock();
            return b + 1;
        }
    }
    unlock();
    return -1;
}

long long ClientKnowledgeBase::computeFileOffset(std::string filename, int partitionNb, int block, bool completedFile) {
    int firstPartition = INT_MAX;
    //int lastPartition = -1;

    if (completedFile) {
        return partitionNb * PARTITION_SIZE + block * BLOCK_SIZE;
    }

    lock();

    std::vector<int> parts = mPartitionsInProgress[filename];
    for (unsigned int i = 0; i < parts.size(); ++i) {
        if (parts.at(i) < firstPartition) {
            firstPartition = parts.at(i);
        }
        //if (parts.at(i) > lastPartition) {
        //    lastPartition = parts.at(i);
        //}
    }
    unlock();
    return (partitionNb - firstPartition) * PARTITION_SIZE + block * BLOCK_SIZE;
}
