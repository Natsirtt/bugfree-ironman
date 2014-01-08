
#include "ClientKnowledgeBase.hpp"

#include <climits>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "Defines.hpp"
#include "Packets/AlivePacket.hpp"
#include "AnswerQueue.hpp"


bool __strVectContains(std::vector<std::string> v, std::string str) {
    for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
        if (str == *it) {
            return true;
        }
    }
    return false;
}

ClientKnowledgeBase::ClientKnowledgeBase() {
    pthread_mutex_init (&mMutex, NULL);

    //Récupération des métadata pour tous les fichiers qui en ont
    //On garde en mémoire les filename qui ont bien une métadata
    //et on crééra de nouvelles métadata pour les fichiers restants
    std::vector<std::string> filesWithMetadata;
    struct dirent* entry;
    DIR* dp;

    dp = opendir(FILES_PATH);
    if (dp == NULL) {
        perror("opendir");
        throw std::runtime_error("Erreur lors de l'ouverture du répertoire");
    }

    while((entry = readdir(dp))) {
        std::string entryName(entry->d_name);
        if ((entry->d_name[0] == '.') && (entryName != std::string(".")) && entryName != std::string("..")) {
            ClientFile cf(entryName);
            mFiles[cf.getName()] = cf;
            filesWithMetadata.push_back(cf.getName());
        }
    }

    closedir(dp);

    dp = opendir(FILES_PATH);
    if (dp == NULL) {
        perror("opendir");
        throw std::runtime_error("Erreur lors de l'ouverture du répertoire");
    }

    while ((entry = readdir(dp))) {
        if (entry->d_name[0] != '.') {
            if (!__strVectContains(filesWithMetadata, std::string(entry->d_name))) {
                std::cout << "New metadata !" << std::endl;
                //Le fichier n'a pas de métadata
                //On récupère sa taille
                std::fstream file((std::string(FILES_PATH) + std::string(entry->d_name)).c_str(), std::fstream::binary | std::fstream::in);
                if (!file.good()) {
                    std::cerr << "FILE NOT GOOD !!" << std::endl;
                    throw std::runtime_error("File not good at open");
                }
                file.seekg(0, file.end);
                long long len = file.tellg();
                file.close();
                ClientFile cf(std::string(entry->d_name), len);
                mFiles[cf.getName()] = cf;
            }
        }
    }
    closedir(dp);
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
    lock();
    mFiles[filename].setBlockData(partition, block, data);
    unlock();
}

int ClientKnowledgeBase::getNextFreeBlockNumber(std::string filename, int partition) {
    return mFiles[filename].getNextFreeBlockNumber(partition);
}

long long ClientKnowledgeBase::getFileSize(std::string filename) {
    if (mFiles[filename].isCorrectFile()) {
        return mFiles[filename].getSize();
    }
    return -1;
}

void ClientKnowledgeBase::shutdown() {
    for (std::map<std::string, ClientFile>::iterator it = mFiles.begin(); it != mFiles.end(); ++it) {
        it->second.serialize();
    }
}

std::vector<ClientFile*> ClientKnowledgeBase::getFiles() {
    std::vector<ClientFile*> res;
    lock();
    for (std::map<std::string, ClientFile>::iterator it = mFiles.begin(); it != mFiles.end(); ++it) {
        res.push_back(&(it->second));
    }
    unlock();
    return res;
}

void ClientKnowledgeBase::sendAll(std::string& trackerIP) {
    lock();
    for (std::map<std::string, ClientFile>::iterator it = mFiles.begin(); it != mFiles.end(); ++it) {
        it->second.send(trackerIP);
    }
    unlock();
}

void ClientKnowledgeBase::sendAlive(std::string& trackerIP) {
    IPacket* packet = new AlivePacket();
    AnswerQueue::get().sendToTracker(packet, trackerIP);
}

void ClientKnowledgeBase::addClientFile(ClientFile cf) {
    mFiles[cf.getName()] = cf;
}
