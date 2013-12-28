#ifndef FILE_HPP
#define FILE_HPP

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

		std::string getKey();

	protected:


	private:
		std::string mName;
		long long mSize;
		int mPartitionsNb;
		int mPartitionSize;
};


#endif // FILE_HPP

