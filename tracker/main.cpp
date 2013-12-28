#include <iostream>

 #include "File.hpp"
 #include "KnowledgeBase.hpp"
 #include "SocketUDP.hpp"
 #include "Operation.hpp"
 #include "NetworkTranslator.hpp"
 #include "OperationQueue.hpp"
 #include "Thread.hpp"

using namespace std;

// Le nombre de threads de traitement qu'on utilise
#define THREAD_NUMBER 5

#define TRACKER_PORT 98765

int main() {

    try {

        // Un appel � la file d'op�rations (l'oblige � se constuire si pas encore fait)
        OperationQueue::get();

        // On genere les threads qui traiteront les operations
        std::vector<Thread> threads;
        for (int i = 0; i < THREAD_NUMBER; ++i) {
            threads.push_back(Thread());
        }

        // On construit la socket principale de reception de packets
        SocketUDP mainSocket("", TRACKER_PORT);

        NetworkTranslator nt(&mainSocket);

        while (1) { // TODO Faire une condition d'arret propre
            IPacket* packet = NULL;
            std::string adresse;
            int port = -1;

            // On r�cup�re un paquet � partir de la socket
            packet = nt.readPacket(adresse, &port, 0);

            // On construit une op�ration
            Operation op(packet, adresse, port);

            // On ajoute l'op�ration � la file des op�rations
            OperationQueue::get().addOperation(op);

        }

        // On nettoie toutes les donn�es
        mainSocket.close();
        for (unsigned int i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
        OperationQueue::get().clear();

    } catch(std::exception& e) {
        std::cerr << "Une erreur est survenue : " << e.what() << std::endl;
    }

    return 0;
}
