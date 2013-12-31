#ifndef WRQPACKET_HPP
#define WRQPACKET_HPP

#include "IPacket.hpp"

/**
 * Organisation d'un WRQ :
 * -----------------------------
 * |opcode|filename|partitionNb|
 * -----------------------------
 * | 4    | 100    | 4         |tailles en octets
 * -----------------------------
 * Taille d'un RRQ : 108 octets
 */
class WRQPacket : public IPacket {
    public:
        static const int MAX_FILENAME_SIZE = 100;

        /**
         * Construit un paquet � partir de ces param�tres.
         */
        WRQPacket(std::string filename, int partition);

        /**
         * Reconstruit un paquet � partir des donn�es au format r�seau.
         */
        WRQPacket(char* data, int size);

        ~WRQPacket();

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

        //Accesseurs

        std::string getName();

        int getPartitionNb();

    private:
        std::string mFileName;
        int mPartition;
};

#endif // IPACKET_HPP

