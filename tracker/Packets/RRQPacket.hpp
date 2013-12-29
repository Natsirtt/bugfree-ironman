#ifndef RRQPACKET_HPP
#define RRQPACKET_HPP

#include "IPacket.hpp"

class RRQPacket : public IPacket {
    public:
        /**
         * Construit un paquet à partir de ces paramétres.
         */
        RRQPacket(std::string filename, int partition);

        /**
         * Reconstruit un paquet à partir des données au format réseau.
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
};

#endif // IPACKET_HPP

