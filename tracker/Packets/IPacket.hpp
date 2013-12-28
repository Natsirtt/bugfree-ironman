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
         * Renvoie le paquet sous la forme d'un tableau de char dans le format réseau.
         */
        virtual char* toData() = 0;

        /**
         * Execute la commande associée à ce paquet.
         * @param adresse L'adresse de la personne ayant envoyé ce paquet.
         */
         virtual void exec(std::string adresse) = 0;
};

#endif // IPACKET_HPP
