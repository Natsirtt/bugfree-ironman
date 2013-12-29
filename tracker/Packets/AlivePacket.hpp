#ifndef ALIVEPACKET_HPP
#define ALIVEPACKET_HPP

#include "IPacket.hpp"

/**
 * Organisation d'un Alive :
 * --------
 * |opcode|
 * --------
 * | 4    | taille en octets
 * --------
 * Taille d'un Alive : 4 octets
 */
class AlivePacket : public IPacket {
    public:
        /**
         * Construit un paquet � partir de ces param�tres.
         */
        AlivePacket();

        /**
         * Reconstruit un paquet � partir des donn�es au format r�seau.
         */
        AlivePacket(char* data, int size);

        ~AlivePacket();

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
};

#endif // IPACKET_HPP

