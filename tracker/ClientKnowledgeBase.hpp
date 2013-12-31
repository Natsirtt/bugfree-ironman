#ifndef CLIENTKNOWLEDGEBASE_HPP
#define CLIENTKNOWLEDGEBASE_HPP

#include <map>
#include <vector>
#include <string>
#include <pthread.h>

class ClientKnowledgeBase {

    public:
        //Accesseur singleton
        static ClientKnowledgeBase& get() {
            static ClientKnowledgeBase kb;
            return kb;
        }

        std::vector<int> getPartitions(std::string filename);
        std::vector<int> getBlocks(std::string filename, int partitionNb);

        bool hasPartition(std::string filename, int partitionNb);
        bool hasBlock(std::string filename, int partitionNb, int block);

        void addPartition(std::string filename, int partitionNb);
        void addBlock(std::string filename, int partitionNb, int block);

    protected:

    private:
        ClientKnowledgeBase();

        //Association filename -> liste des partitions possédées
        std::map<std::string, std::vector<int> > mPartitions;
        //Association filename + partitionNb -> blocks reçus
        std::map<std::string, std::vector<int> > mBlocks;
        //Mutex pour rendre l'instance thread safe
        pthread_mutex_t mMutex;
};


#endif // CLIENTKNOWLEDGEBASE_HPP

