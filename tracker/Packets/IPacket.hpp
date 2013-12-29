#ifndef IPACKET_HPP
#define IPACKET_HPP

#include <string>

class IPacket {

    public:
        virtual ~IPacket() {};

        /**
         * Renvoie l'opcode du packet.
         */
        virtual int getOpcode() = 0;

        /**
         * Renvoie la taille du paquet.
         */
        virtual int getSize() = 0;

        /**
         * Renvoie le paquet sous la forme d'un tableau de char dans le format réseau.
         * Note : Les 4 premiers octets sont un entier représentant un opcode.
         */
        virtual char* toData() = 0;

        /**
         * Execute la commande associée à ce paquet.
         * @param adresse L'adresse de la personne ayant envoyé ce paquet.
         */
         virtual void exec(std::string adresse) = 0;
};

#endif // IPACKET_HPP
