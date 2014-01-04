#include "ClientKnowledgeBase.hpp"
#include <stdexcept>
#include <sstream>

ClientKnowledgeBase::ClientKnowledgeBase() {
    pthread_mutex_init (&mMutex, NULL);
}

void ClientKnowledgeBase::lock() {
    if (pthread_mutex_lock(&mMutex) !=0) {
        throw std::runtime_error("Erreur lors du lock du mutex de la ClientKnowledgeBase");
    }
}

void ClientKnowledgeBase::unlock() {
    if (pthread_mutex_unlock(&mMutex) !=0) {
        throw std::runtime_error("Erreur lors du unlock du mutex de la ClientKnowledgeBase");
    }
}

std::vector<int> ClientKnowledgeBase::getPartitions(std::string filename) {
    lock();
    std::vector<int> res = mPartitions[filename];
    pthread_mutex_unlock(&mMutex);
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

std::vector<char> ClientKnowledgeBase::getBlockData(std::string filename, int partition, int block) {
    // TODO
    return std::vector<char>();
}

void ClientKnowledgeBase::setBlockData(std::string filename, int partition, int block, std::vector<char> data) {
    // TODO
}
