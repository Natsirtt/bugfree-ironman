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
         * Construit un paquet à partir de ces paramétres.
         */
        WRQPacket(std::string filename, int partition);

        /**
         * Reconstruit un paquet à partir des données au format réseau.
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
         * Renvoie le paquet sous la forme d'un tableau de char dans le format réseau.
         * Note : Les 4 premiers octets sont un entier représentant un opcode.
         */
        char* toData();

        /**
         * Execute la commande associée à ce paquet.
         * @param adresse L'adresse de la personne ayant envoyé ce paquet.
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

