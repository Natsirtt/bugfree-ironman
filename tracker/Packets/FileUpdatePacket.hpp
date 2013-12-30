#ifndef FILEUPDATEPACKET_HPP
#define FILEUPDATEPACKET_HPP

#include "IPacket.hpp"

/**
 * Organisation d'un FileUpdate :
 * -------------------------------------------------------------
 * | opcode | filename | taille bitmap | bitmap des partitions |
 * -------------------------------------------------------------
 * | 4      | 100      | 4             | 512                   | tailles en octets
 * -------------------------------------------------------------
 * Taille d'un FileUpdate : 620 octets
 */
class FileUpdatePacket : public IPacket {
    public:
        static const int MAX_BITMAP_SIZE = 512;
        static const int MAX_FILENAME_SIZE = 100;

        /**
         * Construit un paquet � partir de ces param�tres.
         */
        FileUpdatePacket(std::string filename, char* partitionBitmap);

        /**
         * Reconstruit un paquet � partir des donn�es au format r�seau.
         */
        FileUpdatePacket(char* data, int size);

        ~FileUpdatePacket();

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
        int mBitmapSize;
        char* mPartitionBitmap;
};

#endif //FILEUPDATEPACKET_HPP

