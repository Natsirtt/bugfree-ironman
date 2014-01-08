#ifndef CLIENTFILE_HPP
#define CLIENTFILE_HPP

#include <vector>
#include <string>
#include <map>

class ClientFile {

public:
    ClientFile();
    ClientFile(std::string filename, long long int fileSize, bool init);
    ClientFile(std::string filename);

    std::string getName();

    void serialize();
    void lock();
    void unlock();

    bool hasPartition(int part);
    bool hasBlock(int part, int block);
    bool isPartitionInProgress(int part);
    bool isPartitionAcquiredOrInProgress(int part);
    bool isCorrectFile();

    void beginPartition(int part);
    void endPartition(int part);
    void addBlock(int part, int block);

    std::vector<char> getBlockData(int part, int block);
    void setBlockData(int part, int block, std::vector<char> data);

    int getFirstFreeBit(std::vector<char>& bitmap);
    int getLastFreeBit(std::vector<char>& bitmap);
    int getFirstUsedBit(std::vector<char>& bitmap);
    int getLastUsedBit(std::vector<char>& bitmap);
    int getNextFreeBlockNumber(int part);
    long long getSize();

    /**
     * Envoie le fichier au tracker.
     */
    void send(std::string& trackerIP);

protected:

private:
    void createFile(std::string& filename, long long int size);

    typedef std::vector<char> bitmap_t;
    //La bitmap des partitions possédées
    bitmap_t mPartitions;
    //La bitmap des partitions en cours de récupération
    bitmap_t mPartitionsInProgress;
    //Une map associant à chaque partition une bitmap des blocks récupérés
    std::map<int, bitmap_t> mBlocks;
    //Le mutex du fichier
    pthread_mutex_t mMutex;
    //Le nom du fichier
    std::string mFilename;
    //La taille du fichier final
    long long mFileSize;

    long long computeFileOffset(int part, int block);
    bool isNthBitSet(std::vector<char>& bitmap, int n);
    bool isNthBitSet(char c, int n);
    void setNthBit(std::vector<char> &bitmap, int n);
};

#endif // CLIENTFILE_HPP

