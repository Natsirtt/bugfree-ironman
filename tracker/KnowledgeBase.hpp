#ifndef KNOWLEDGEBASE_HPP
#define KNOWLEDGEBASE_HPP

#include <map>
#include <vector>
#include <string>
#include <set>

#include "File.hpp"
#include "Client.hpp"

class KnowledgeBase {

	public:
		static KnowledgeBase get() {
            static KnowledgeBase kb;
            return kb;
		}

		std::vector<int> getPartitions(std::string client, std::string file);
		std::vector<std::string> getClients(std::string file);
		std::vector<std::string> getClients(std::string file, int partition);

		void addPartition(std::string client, std::string file, int partition);
        void addFile(File file);

        Client& getClient(std::string clientName);

	private:
	    KnowledgeBase();

        typedef std::map<std::string, std::vector<int> > strVectMap;
        // Association client -> liste des fichiers/partitions que possède le client
        std::map<std::string, strVectMap> mClients;
        // Association fichier -> liste de cliens possèdant le fichier
        std::map<std::string, std::set<std::string> > mFiles;
        // Liste des fichiers existants sur le réseau
        std::map<std::string, File> mFilesDesc;
        // Liste des clients existants sur le réseau
        std::map<std::string, Client> mClientDesc;

        bool hasPartition(std::string client, std::string file, int partition);
};


#endif // KNOWLEDGEBASE_HPP
