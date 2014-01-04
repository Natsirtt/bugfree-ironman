#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <vector>
#include <string>
#include <map>

class Client {
    public:
        // Le nombre de secondes à attendre avant de considérer un client déconnecté
        static const int CONNECTION_TIMEOUT = 10;

        Client(std::string adresse);

        std::string getAdresse();
        /**
         * Met à jour la date de dernière connection du client.
         */
        void alive();
        /**
         * Revoie si le client est toujours en vie.
         */
        bool isAlive();
        void updateFile(std::string& filename, char* bitmap, int bitmapSize);
        bool hasFile(std::string& filename);
        bool hasPartition(std::string& filename, int partNb);

        int getPartitionNumber(std::string& filename);

    private:
        std::string mAdresse;
        int mLastAlive;

        std::map<std::string, std::vector<char> > mFiles;
        std::map<std::string, int> mFilesPartitionCount;
};

#endif // CLIENT_HPP
