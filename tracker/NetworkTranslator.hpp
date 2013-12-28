#ifndef NETWORK_TRANSLATOR_HPP
#define NETWORK_TRANSLATOR_HPP


#include "SocketUDP.hpp"
#include "Packets/IPacket.hpp"

/**
 * Classe permettant de reconstruire des paquets � partir d'une socket.
 */
class NetworkTranslator {
    public:
        /**
         * Constructeur.
         * @param socket La socket sur laquelle on va recevoir les donn�es.
         */
        NetworkTranslator(SocketUDP* socket);

        /**
         * Re�oie des paquets.
         * @param adresse L'adresse dont on veut recevoir des donn�es. Vide pour toutes.
         * @param port Le port duquel on veut recevoir des donn�es. -1 pour tous.
         * @param timeout Le timeout de la fonction. 0 Pour fonction bloquante.
         * @return
         *     NULL en cas d'erreur.
         *     adresse et port sont modifi�s si ils �taient non sp�cifi�es.
         */
        IPacket* readPacket(std::string& adresse, int* port, int timeout);


    private:
        SocketUDP* mSocket;
};


#endif // NETWORKTRANSLATOR_HPP
