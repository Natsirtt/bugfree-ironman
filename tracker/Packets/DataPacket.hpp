#ifndef DATAPACKET_HPP
#define DATAPACKET_HPP

#include "IPacket.hpp"

/**
 * Organisation d'un DATA :
 * ---------------------------------------------------------
 * |opcode|filename|partitionNb|blockNb|blockSize|blockData|
 * ---------------------------------------------------------
 * | 4    | 100    | 4         | 4     | 4       | 512     | tailles en octets
 * ---------------------------------------------------------
 * Taille d'un DATA : 628 octets
 */
class DataPacket : public IPacket {
    public:
        static const int MAX_DATA_SIZE = 512;
        static const int MAX_FILENAME_SIZE = 100;

        /**
         * Construit un paquet � partir de ces param�tres.
         */
        DataPacket(std::string filename, int partition, int blockNb, int blockSize, char* blockData);

        /**
         * Reconstruit un paquet � partir des donn�es au format r�seau.
         */
        DataPacket(char* data, int size);

        ~DataPacket();

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
        int mPartition;
        int mBlockNb;
        int mBlockSize;
        char* mBlockData;
};

#endif // IPACKET_HPP

