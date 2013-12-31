#include "ClientKnowledgeBase.hpp"

#include <sstream>

ClientKnowledgeBase::ClientKnowledgeBase() {
    mMutex = PTHREAD_MUTEX_INITIALIZER;
}

void __lock() {
    if (pthread_mutex_lock(&mMutex) !=0) {
        throw std::runtime_error("Erreur lors du lock du mutex de la ClientKnowledgeBase");
    }
}

void __unlock() {
    if (pthread_mutex_unlock(&mMutex) !=0) {
        throw std::runtime_error("Erreur lors du unlock du mutex de la ClientKnowledgeBase");
    }
}

std::vector<int> ClientKnowledgeBase::getPartitions(std::string filename) {
    __lock();
    std::vector res = mPartitions[filename];
    pthread_mutex_unlock(&mMutex);
    __unlock();
    return res;
}

std::string __blocksMapKey(std::string filename, int partitionNb) {
    std::string key = filename;
    std::ostringstream converter;
    converter << partitionNb;
    key += converter.str();
    return key;
}

std::vector<int> ClientKnowledgeBase::getBlocks(std::string filename, int partitionNb) {
    __lock();
    std::vector<int> res = mBlocks[__blocksMapKey(filename, partitionNb)];
    __unlock();
    return res;
}

bool ClientKnowledgeBase::hasPartition(std::string filename, int partitionNb) {
    __lock();
    std::vector<int> partitions = getPartitions(filename);
    for (unsigned int i = 0; i < partitions.size(); ++i) {
        if (partitionNb == partitions.at(i)) {
                __unlock();
            return true;
        }
    }
    __unlock();
    return false;
}

bool ClientKnowledgeBase::hasBlock(std::string filename, int partitionNb, int block) {
    __lock();
    std::vector<int> blocks = getBlocks(filename, partitionNb);
    for (unsigned int i = 0; i < blocks.size(); ++i) {
        if (block == blocks.at(i)) {
            __unlock();
            return true
        }
    }
    __unlock();
    return false;
}

void ClientKnowledgeBase::addPartition(std::string filename, int partitionNb) {
    __lock();
    if (!hasPartition(filename, partitionNb) {
        mPartitions[filename].insert(partitionNb);
        //TODO vider une liste de numéros de block pour une partition complète puisque ça devient inutile ?
    }
    __unlock();
}

void ClientKnowledgeBase::addBlock(std::string filename, int partititonNb, int block) {
    __lock();
    if (!hasBlock(filename, partitionNb, block)) {
        mBlocks[__blocksMapKey(filename, partitionNb)].insert(block);
    }
    __unlock();
}
