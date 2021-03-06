#ifndef FILE_ANWSER_PACKET_HPP
#define FILE_ANWSER_PACKET_HPP


#include <vector>
#include <string>
#include "IPacket.hpp"

#include "../KnowledgeBase.hpp"

/**
 * Organisation d'un FileAnswer :
 * -----------------------------------------------------------------
 * |opcode|filename|envoie?|filesize|assocNb|tableau d'associations|
 * -----------------------------------------------------------------
 * | 4    | 100    | 1     | 8      | 4     | taille d'assoc * 8   | tailles en octets
 * -----------------------------------------------------------------
 *    Association :
 *    --------------------------
 *    | nbPartition | ipClient |
 *    --------------------------
 *    | 4           | 60 ?     |
 *    --------------------------
 * Taille d'un FileAnswer : 629 octets
 */
class FileAnswerPacket : public IPacket {
    public:
        static const int MAX_FILENAME_SIZE = 100;
        static const int MAX_ASSOC_NUMBER = 8;

        /**
         * Construit un paquet � partir de ces param�tres.
         */
        FileAnswerPacket(std::string filename, bool send, long long int filesize, std::vector<Association> assoc);

        /**
         * Reconstruit un paquet � partir des donn�es au format r�seau.
         */
        FileAnswerPacket(char* data, int size);

        ~FileAnswerPacket();

        /**
         * Renvoie l'opcode du packet.
         */
        unsigned int getOpcode();

        /**
         * Renvoie la taille du paquet.
         */
        int getSize();

        /**
         * Renvoie le paquet sous la forme d'un tableau de char dans le format r�seau.
         * Note : Les 4 premiers octets sont un entier repr�sentant un opcode.
         */
        char* toData();

        /**
         * Execute la commande associ�e � ce paquet.
         * @param adresse L'adresse de la personne ayant envoy� ce paquet.
         */
        void exec(std::string adresse);

    private:
        std::string mFileName;
        bool mSend;
        long long int mFilesize;
        std::vector<Association> mAssoc;
};

#endif // IPACKET_HPP

