#include "KnowledgeBase.hpp"

#include <cstring>

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

Client& KnowledgeBase::getClient(std::string clientName) {
    try {
        Client c = mClientDesc.at(clientName);
    } catch (...) {
        Client client(clientName);
        mClientDesc.insert(std::pair<std::string, Client>(clientName, client));
    }
    return mClientDesc.at(clientName);
}

int KnowledgeBase::getConnectedClientCount() {
    int count = 0;
    for (std::map<std::string, Client>::iterator it = mClientDesc.begin(); it != mClientDesc.end(); ++it) {
        if (it->second.isAlive()) {
            count++;
        }
    }
    return count;
}

File& KnowledgeBase::getFile(std::string fileName) {
    try {
        File f = mFilesDesc.at(fileName);
    } catch (...) {
        //File file(fileName); TODO ajouter les paramètre manquants
        //mFilesDesc.insert(std::pair<std::string, Client>(fileName, file));
    }
    return mFilesDesc.at(fileName);
}

std::vector<Association> KnowledgeBase::getClientsToSend(std::string filename) {
    std::vector<Association> assocs;

    int connectedClient = getConnectedClientCount();
    File& f = getFile(filename);

    int ratio =  std::max(1, f.getPartitionsNb() / connectedClient);
    if ((f.getPartitionsNb() % connectedClient) != 0) {
        ratio++;
    }

    for (int i = 0; i < f.getPartitionsNb(); ++i) {
        if (f.getClients(i).size() == 0) {
            for (std::map<std::string, Client>::iterator it = mClientDesc.begin(); it != mClientDesc.end(); ++it) {
                if (it->second.isAlive() && (it->second.getPartitionNumber(filename) < ratio)) {
                    Association assoc;
                    assoc.partition = i;
                    strncpy(assoc.ipClient, it->second.getAdresse().c_str(), 60);
                    assocs.push_back(assoc);
                    break;
                }
            }
        }
    }

    return assocs;
}
