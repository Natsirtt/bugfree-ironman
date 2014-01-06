#ifndef FILE_REQUEST_PACKET_HPP
#define FILE_REQUEST_PACKET_HPP

#include "IPacket.hpp"

/**
 * Organisation d'un FileRequest :
 * ----------------------------------
 * |opcode|filename|envoie?|filesize|
 * ----------------------------------
 * | 4    | 100    | 1     | 8      |
 * ----------------------------------
 * Taille d'un FileRequest : 113 octets
 */
class FileRequestPacket : public IPacket {
    public:
        static const int MAX_FILENAME_SIZE = 100;

        /**
         * Construit un paquet � partir de ces param�tres.
         * @param send true si l'on veux envoyer le fichier.
         * @param filesize Si (send == true) => la taille du fichier.
         */
        FileRequestPacket(std::string filename, bool send, long long int filesize);

        /**
         * Reconstruit un paquet � partir des donn�es au format r�seau.
         */
        FileRequestPacket(char* data, int size);

        ~FileRequestPacket();

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
};

#endif // IPACKET_HPP

