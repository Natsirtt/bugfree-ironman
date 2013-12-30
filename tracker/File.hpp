#ifndef FILE_HPP
#define FILE_HPP

#include <set>
#include <string>

class File {
	public:
        File();
		File(std::string name, long long size, int partitionSize);

		std::string getName();
		long long getSize();
		int getPartitionsNb();
		int getPartitionSize();
		int getLastPartitionSize();

        void addClient(std::string clientName);
		std::set<std::string>& getClients();

		std::string getKey();

	protected:


	private:
		std::string mName;
		long long mSize;
		int mPartitionsNb;
		int mPartitionSize;

		std::set<std::string> mClients;
};


#endif // FILE_HPP

