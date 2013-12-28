#include "KnowledgeBase.hpp"

KnowledgeBase::KnowledgeBase() {}

std::vector<int> KnowledgeBase::getPartitions(std::string client, std::string file) {
    return mClients[client][file];
}

std::vector<std::string> KnowledgeBase::getClients(std::string file) {
    std::set<std::string> potentialClients = mFiles[file];
    std::vector<std::string> clients;
    std::set<std::string>::iterator it = potentialClients.begin();
    for (; it != potentialClients.end(); ++it) {
        clients.push_back(*it);
    }
    return clients;
}

std::vector<std::string> KnowledgeBase::getClients(std::string file, int partition) {
    std::set<std::string> potentialClients = mFiles[file];
    std::vector<std::string> clients;
    std::set<std::string>::iterator it = potentialClients.begin();
    for (; it != potentialClients.end(); ++it) {
        if (hasPartition(*it, file, partition)) {
            clients.push_back(*it);
        }
    }
    return clients;
}

void KnowledgeBase::addPartition(std::string client, std::string file, int partition) {
    mClients[client][file].push_back(partition);
    mFiles[file].insert(client);
}

void KnowledgeBase::addFile(File file) {
    mFilesDesc[file.getKey()] = file;
}

bool KnowledgeBase::hasPartition(std::string client, std::string file, int partition) {
    std::vector<int> partitions = getPartitions(client, file);
    for (unsigned int i = 0; i < partitions.size(); ++i) {
        if (partition == partitions.at(i)) {
            return true;
        }
    }
    return false;
}