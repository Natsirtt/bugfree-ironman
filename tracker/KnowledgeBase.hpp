#ifndef KNOWLEDGEBASE_HPP
#define KNOWLEDGEBASE_HPP

#include <map>
#include <vector>
#include <string>
#include <set>

#include "File.hpp"
#include "Client.hpp"
#include "Association.hpp"

/**
 * Classe permettant de garder toutes les données du tracker.
 * Note : ThreadSafe
 */
class KnowledgeBase { // TODO rendre threadSafe

	public:
		static KnowledgeBase& get() {
            static KnowledgeBase kb;
            return kb;
		}
        /**
         * Renvoie les partitions d'un client pour un fichier donné.
         */
		std::vector<int> getPartitions(std::string client, std::string file);
        /**
         * Renvoie les clients possédant au moins une partition d'un fichier donné.
         */
		std::vector<std::string> getClients(std::string file);
        /**
         * Renvoie les clients possédant une partition d'un fichier donné.
         */
		std::vector<std::string> getClients(std::string file, int partition);

		void addPartition(std::string client, std::string file, int partition);
        void addFile(File file);

        Client& getClient(std::string clientName);
        int getConnectedClientCount();

        /**
         * Note : Renvoie une exception si le fichier n'existe pas.
         */
        File& getFile(std::string fileName);

        /**
         * Renvoie les clients auquels il faut envoyer un fichier.
         * Permet de calculer les clients les mieux placés pour recevoir une partition.
         */
        std::vector<Association> getClientsToSend(std::string& filename, std::string& adresse);

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
