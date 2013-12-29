#ifndef RRQPACKET_HPP
#define RRQPACKET_HPP

#include "IPacket.hpp"

class RRQPacket : public IPacket {
    public:
        /**
         * Construit un paquet � partir de ces param�tres.
         */
        RRQPacket(std::string filename, int partition);

        /**
         * Reconstruit un paquet � partir des donn�es au format r�seau.
         */
        RRQPacket(char* data);

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
};

#endif // IPACKET_HPP

