#include "File.hpp"

#include <cmath>
#include <cstring>

#include "Client.hpp"

File::File() : mName(""), mSize(0), mPartitionsNb(0), mPartitionSize(0) {
}

File::File(std::string& name, long long size, int partitionSize) : mName(name), mSize(size), mPartitionSize(partitionSize) {
    mPartitionsNb = size / partitionSize;
    if (size % partitionSize != 0) {
        mPartitionsNb++;
    }
}

std::string File::getName() {
    return mName;
}

long long File::getSize() {
    return mSize;
}

int File::getPartitionSize() {
    return mPartitionSize;
}

int File::getLastPartitionSize() {
    int size = mSize % mPartitionSize;
    if (size == 0) {
        return mPartitionSize;
    }
    return size;
}

int File::getPartitionsNb() {
    return mPartitionsNb;
}

std::string File::getKey() {
    return mName;
}

void File::addClient(Client* client, int partition) {
    mClients.insert(client);

    mPartitionClient[partition].insert(client);
}

std::set<Client*>& File::getClients() {
    return mClients;
}

std::set<Client*>& File::getClients(int partition) {
    return mPartitionClient[partition];
}

std::vector<Association> File::getClientsToAsk(std::string& adresse) {
    std::vector<Association> assocs;
    for (int i = 0; i < mPartitionsNb; ++i) {
        std::set<Client*>& clients = mPartitionClient[i];

        std::set<Client*>::iterator it;
        for (it = clients.begin(); it != clients.end(); ++it) {
            if ((*it)->isAlive() && ((*it)->getAdresse() != adresse)) {
                Association assoc;
                assoc.partition = i;
                strncpy(assoc.ipClient, (*it)->getAdresse().c_str(), 60);
                assoc.ipClient[59] = '\0';
                assocs.push_back(assoc);
                break;
            }
        }
    }
    return assocs;
}

