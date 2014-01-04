#ifndef FILE_HPP
#define FILE_HPP

#include <set>
#include <string>
#include <map>
#include <vector>

#include "Association.hpp"

class Client;

class File {
	public:
        File();
		File(std::string& name, long long size, int partitionSize);

		std::string getName();
		long long getSize();
		int getPartitionsNb();
		int getPartitionSize();
		int getLastPartitionSize();

        void addClient(Client* client, int partition);
		std::set<Client*>& getClients();
		std::set<Client*>& getClients(int partition);

		std::vector<Association> getClientsToAsk();

		std::string getKey();

	private:
		std::string mName;
		long long mSize;
		int mPartitionsNb;
		int mPartitionSize;
        // Ensemble de tous les clients qui possèdent au moins une partition
		std::set<Client*> mClients;
        // Association entre les partitions et les ensembles de clients les contenant
		std::map<int, std::set<Client*> > mPartitionClient;
};


#endif // FILE_HPP

