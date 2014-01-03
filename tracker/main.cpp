#include <iostream>

 #include "File.hpp"
 #include "KnowledgeBase.hpp"
 #include "SocketUDP.hpp"
 #include "Operation.hpp"
 #include "NetworkTranslator.hpp"
 #include "OperationQueue.hpp"
 #include "Thread.hpp"
 #include "Defines.hpp"
 #include "AnswerQueue.hpp"

using namespace std;


int main() {
    try {
        // Un appel à la file d'opérations (l'oblige à se constuire si pas encore fait)
        OperationQueue::get();

        // Un appel à la file des réponses (l'oblige à se constuire si pas encore fait) et démarre de thread
        AnswerQueue::get().start();

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

            // On récupére un paquet à partir de la socket
            packet = nt.readPacket(adresse, &port, 0);

            // On construit une opération
            Operation op(packet, adresse, port);

            // On ajoute l'opération à la file des opérations
            OperationQueue::get().addOperation(op);
        }

        // On nettoie toutes les données
        mainSocket.close();
        for (unsigned int i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
        OperationQueue::get().clear();
        AnswerQueue::get().stop();
    } catch(std::exception& e) {
        std::cerr << "Une erreur est survenue : " << e.what() << std::endl;
    }

    return 0;
}
