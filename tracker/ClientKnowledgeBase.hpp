#ifndef CLIENTKNOWLEDGEBASE_HPP
#define CLIENTKNOWLEDGEBASE_HPP

#include <map>
#include <vector>
#include <string>
#include <pthread.h>

#include "ClientFile.hpp"

class ClientKnowledgeBase {

    public:
        //Accesseur singleton
        static ClientKnowledgeBase& get() {
            static ClientKnowledgeBase kb;
            return kb;
        }

        //std::vector<int> getPartitions(std::string filename);
        //std::vector<int> getBlocks(std::string filename, int partitionNb);
        //std::vector<int> getPartitionsInProgress(std::string filename);

        bool hasPartition(std::string filename, int partitionNb);
        bool hasBlock(std::string filename, int partitionNb, int block);
        bool isPartitionInProgress(std::string filename, int partitionNb);

        void addPartition(std::string filename, int partitionNb);
        void beginPartition(std::string filename, int partitionNb);

        std::vector<char> getBlockData(std::string filename, int partition, int block);
        void setBlockData(std::string filename, int partition, int block, std::vector<char> data);

        int getNextFreeBlockNumber(std::string filename, int partition);
        long long getFileSize(std::string filename);

        void shutdown();

        /**
         * Renvoie la liste de tous les fichiers dans la base.
         */
        std::vector<ClientFile*> getFiles();

        /**
         * Envoie les données à jour au tracker.
         */
        void sendAll(std::string& trackerIP);

        void sendAlive(std::string& trackerIP);

        void addClientFile(ClientFile cf);

    protected:

    private:
        ClientKnowledgeBase();

        void lock();
        void unlock();

        void addBlock(std::string filename, int partitionNb, int block);

        //L'association filename -> objet file
        std::map<std::string, ClientFile> mFiles;
        //Mutex pour rendre l'instance thread safe
        pthread_mutex_t mMutex;

};


#endif // CLIENTKNOWLEDGEBASE_HPP

