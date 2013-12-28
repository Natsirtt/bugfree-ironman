#ifndef IPACKET_HPP
#define IPACKET_HPP

class IPacket {

    public:
        virtual ~IPacket() = 0;

        /**
         * Renvoie l'opcode du packet.
         */
        virtual int getOpcode() = 0;

        /**
         * Renvoie la tailel du paquet.
         */
        virtual int getSize() = 0;

        /**
         * Renvoie le paquet sous la forme d'un tableau de char dans le format r�seau.
         */
        virtual char* toData() = 0;
};

#endif // IPACKET_HPP
