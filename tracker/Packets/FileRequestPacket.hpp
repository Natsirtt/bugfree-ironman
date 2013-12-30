#ifndef FILE_REQUEST_PACKET_HPP
#define FILE_REQUEST_PACKET_HPP

#include "IPacket.hpp"

/**
 * Organisation d'un FileRequest :
 * -------------------------
 * |opcode|filename|envoie?|
 * -------------------------
 * | 4    | 100    | 1     |
 * -------------------------
 * Taille d'un FileRequest : 105 octets
 */
class FileRequestPacket : public IPacket {
    public:
        static const int MAX_FILENAME_SIZE = 100;

        /**
         * Construit un paquet � partir de ces param�tres.
         * @param send true si l'on veux envoyer le fichier.
         */
        FileRequestPacket(std::string filename, bool send);

        /**
         * Reconstruit un paquet � partir des donn�es au format r�seau.
         */
        FileRequestPacket(char* data, int size);

        ~FileRequestPacket();

        /**
         * Renvoie l'opcode du packet.
         */
        int getOpcode();

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
};

#endif // IPACKET_HPP

