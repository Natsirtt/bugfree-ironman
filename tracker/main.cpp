#include <iostream>

 #include "File.hpp"
 #include "KnowledgeBase.hpp"
 #include "SocketUDP.hpp"
 #include "Operation.hpp"
 #include "NetworkTranslator.hpp"
 #include "OperationQueue.hpp"

using namespace std;

#define TRACKER_PORT 98765

int main() {

    // Un appel à la file d'opérations (l'oblige à se constuire si pas encore fait)
    OperationQueue::get();

    // On genere les threads qui traiteront les operations


    // On construit la socket principale de reception de packets
    SocketUDP mainSocket("", TRACKER_PORT);

    NetworkTranslator nt(&mainSocket);

    while (1) { // TODO Faire une condition d'arret propre
        IPacket* packet = NULL;
        std::string adresse;
        int port = -1;

        // On récupére un packet à partir de la socket
        packet = nt.readPacket(adresse, &port, 0);

        // On construit une opération
        Operation op(packet, adresse, port);

        // On ajoute l'opération à la file des opérations
        OperationQueue::get().addOperation(op);

    }

    // On nettoie toutes les données
    mainSocket.close();

    return 0;
}
