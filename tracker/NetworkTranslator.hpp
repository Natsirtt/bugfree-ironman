#ifndef NETWORK_TRANSLATOR_HPP
#define NETWORK_TRANSLATOR_HPP


#include "SocketUDP.hpp"
#include "Packets/IPacket.hpp"

/**
 * Classe permettant de reconstruire des paquets à partir d'une socket.
 */
class NetworkTranslator {
    public:
        /**
         * Constructeur.
         * @param socket La socket sur laquelle on va recevoir les données.
         */
        NetworkTranslator(SocketUDP* socket);

        /**
         * Reçoie des paquets.
         * @param adresse L'adresse dont on veut recevoir des données. Vide pour toutes.
         * @param port Le port duquel on veut recevoir des données. -1 pour tous.
         * @param timeout Le timeout de la fonction. 0 Pour fonction bloquante.
         * @return
         *     NULL en cas d'erreur.
         *     adresse et port sont modifiés si ils étaient non spécifiées.
         */
        IPacket* readPacket(std::string& adresse, int* port, int timeout);


    private:
        SocketUDP* mSocket;
};


#endif // NETWORKTRANSLATOR_HPP
