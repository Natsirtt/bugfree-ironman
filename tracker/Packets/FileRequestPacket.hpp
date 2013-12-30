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
         * Construit un paquet à partir de ces paramétres.
         * @param send true si l'on veux envoyer le fichier.
         */
        FileRequestPacket(std::string filename, bool send);

        /**
         * Reconstruit un paquet à partir des données au format réseau.
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
         * Renvoie le paquet sous la forme d'un tableau de char dans le format réseau.
         * Note : Les 4 premiers octets sont un entier représentant un opcode.
         */
        char* toData();

        /**
         * Execute la commande associée à ce paquet.
         * @param adresse L'adresse de la personne ayant envoyé ce paquet.
         */
        void exec(std::string adresse);

    private:
        std::string mFileName;
        bool mSend;
};

#endif // IPACKET_HPP

