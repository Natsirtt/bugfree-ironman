#ifndef RRQPACKET_HPP
#define RRQPACKET_HPP

#include "IPacket.hpp"

/**
 * Organisation d'un RRQ :
 * -----------------------------------------
 * |opcode|filename|partitionNb|firstPacket|
 * -----------------------------------------
 * | 4    | 100    | 4         | 4         |tailles en octets
 * -----------------------------------------
 * Taille d'un RRQ : 112 octets
 */
class RRQPacket : public IPacket {
    public:
        static const int MAX_FILENAME_SIZE = 100;

        /**
         * Construit un paquet à partir de ces paramétres.
         */
        RRQPacket(std::string filename, int partition, int firstBlock);

        /**
         * Reconstruit un paquet à partir des données au format réseau.
         */
        RRQPacket(char* data, int size);

        ~RRQPacket();

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
        int mPartition;
        int mFirstPacket;
};

#endif // IPACKET_HPP

