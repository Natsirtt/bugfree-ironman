
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

bool ClientKnowledgeBase::hasPartition(std::string filename, int partitionNb) {
    return mFiles[filename].hasPartition(partitionNb);
}

bool ClientKnowledgeBase::isPartitionInProgress(std::string filename, int partitionNb) {
    return mFiles[filename].isPartitionInProgress(partitionNb);
}

bool ClientKnowledgeBase::hasBlock(std::string filename, int partitionNb, int block) {
    return mFiles[filename].hasBlock(partitionNb, block);
}

void ClientKnowledgeBase::addPartition(std::string filename, int partitionNb) {
    mFiles[filename].endPartition(partitionNb);
}

void ClientKnowledgeBase::addBlock(std::string filename, int partitionNb, int block) {
    mFiles[filename].addBlock(partitionNb, block);
}

void ClientKnowledgeBase::beginPartition(std::string filename, int partitionNb) {
    mFiles[filename].beginPartition(partitionNb);
}

std::vector<char> ClientKnowledgeBase::getBlockData(std::string filename, int partition, int block) {
    return mFiles[filename].getBlockData(partition, block);
}

void ClientKnowledgeBase::setBlockData(std::string filename, int partition, int block, std::vector<char> data) {
    mFiles[filename].setBlockData(partition, block, data);
}

int ClientKnowledgeBase::getNextFreeBlockNumber(std::string filename, int partition) {
    return mFiles[filename].getNextFreeBlockNumber(partition);
}


