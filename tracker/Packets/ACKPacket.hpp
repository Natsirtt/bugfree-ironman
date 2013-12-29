#ifndef ACKPACKET_HPP
#define ACKPACKET_HPP

#include "IPacket.hpp"

/**
 * Organisation d'un ACK :
 * -----------------------------------------------
 * |opcode|filename|partitionNb|blockNb|nextBlock|
 * -----------------------------------------------
 * | 4    | 100    | 4         | 4     | 4       |tailles en octets
 * -----------------------------------------------
 * Taille d'un ACK : 116 octets
 */
class ACKPacket : public IPacket {
    public:
        static const int MAX_FILENAME_SIZE = 100;

        /**
         * Construit un paquet à partir de ces paramétres.
         */
        ACKPacket(std::string filename, int partition, int blockNb, int nextBlock);

        /**
         * Reconstruit un paquet à partir des données au format réseau.
         */
        ACKPacket(char* data, int size);

        ~ACKPacket();

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
        int mBlockNb;
        int mNextBlock;
};

#endif // IPACKET_HPP

