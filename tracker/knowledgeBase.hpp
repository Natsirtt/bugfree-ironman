#ifndef KNOWLEDGEBASE_HPP
#define KNOWLEDGEBASE_HPP

#include <map>
#include <vector>
#include <string>
#include <set>

#include "file.hpp"

class KnowledgeBase {

	public:
		KnowledgeBase();

		std::vector<int> getPartitions(std::string client, std::string file);
		std::vector<std::string> getClients(std::string file);
		std::vector<std::string> getClients(std::string file, int partition);

		void addPartition(std::string client, std::string file, int partition);
        void addFile(File file);

	protected:


	private:
        typedef std::map<std::string, std::vector<int> > strVectMap;
        //Association client -> liste des fichiers/partitions que possède le client
        std::map<std::string, strVectMap> mClients;
        //Association fichier -> liste de cliens possèdant le fichier
        std::map<std::string, std::set<std::string> > mFiles;
        //Liste des fichiers existants sur le réseau
        std::map<std::string, File> mFilesDesc;

        bool hasPartition(std::string client, std::string file, int partition);
};


#endif // KNOWLEDGEBASE_HPP
