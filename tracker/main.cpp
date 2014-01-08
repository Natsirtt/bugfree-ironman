#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>


#include "File.hpp"
#include "KnowledgeBase.hpp"
#include "SocketUDP.hpp"
#include "Operation.hpp"
#include "NetworkTranslator.hpp"
#include "OperationQueue.hpp"
#include "Thread.hpp"
#include "Defines.hpp"
#include "AnswerQueue.hpp"
#include "ClientKnowledgeBase.hpp"
#include "KnowledgeBase.hpp"
#include "Interface/Interface.hpp"

using namespace std;

void help() {
    std::cout << "Erreur d'utilisation." << std::endl;
    std::cout << "  ./prog CLIENT|TRACKER [ip_tracker] [nb_threads]" << std::endl;
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    try {
        int port = TRACKER_PORT;
        std::string trackerIP("127.0.0.1");
        int nbThreads = THREAD_NUMBER;
        try {
            if ((argc > 1) && (strcmp(argv[1], "CLIENT") == 0)) {
                 port = CLIENT_PORT;
                if (argc > 2) {
                    trackerIP = std::string(argv[2]);
                    if (argc > 3) {
                        std::stringstream ss(argv[3]);
                        ss >> nbThreads;
                    }
                } else {
                    help();
                }
            } else if ((argc > 1) && (strcmp(argv[1], "TRACKER") == 0)) {
                 port = TRACKER_PORT;
            } else {
                help();
            }
        } catch (...) {
            help();
        }

        State::get().setTrackerIp(trackerIP);

        State::get();

        // Un appel à la file d'opérations (l'oblige à se constuire si pas encore fait)
        OperationQueue::get();

        // Un appel à la file des réponses (l'oblige à se constuire si pas encore fait) et démarre de thread
        AnswerQueue::get();
        AnswerQueue::get().start();

        // On genere les threads qui traiteront les operations
        std::vector<Thread> threads;
        for (int i = 0; i < nbThreads; ++i) {
            threads.push_back(Thread());
        }

        // On construit la socket principale de reception de packets
        SocketUDP mainSocket("", port);

        NetworkTranslator nt(&mainSocket);

        if (port == TRACKER_PORT) {
            std::cout << "Demarrage en mode Tracker sur le port : " << port << std::endl;
            KnowledgeBase::get();
        } else {
            std::cout << "Demarrage en mode client sur le port : " << port << " vers le tracker : " << trackerIP << std::endl;
            ClientKnowledgeBase::get();
            //Synchronisation avec le tracker
            ClientKnowledgeBase::get().sendAlive(trackerIP);
            ClientKnowledgeBase::get().sendAll(trackerIP);
            //Interface::get().start();
        }

        while (State::get().isRunning()) {
            IPacket* packet = NULL;
            std::string adresse;
            int port = -1;
            // On récupére un paquet à partir de la socket
            packet = nt.readPacket(adresse, &port, 0);
            if (packet == NULL) {
                continue;
            }

            // On construit une opération
            Operation op(packet, adresse, port);

            // On ajoute l'opération à la file des opérations
            OperationQueue::get().addOperation(op);
        }

        if (port != TRACKER_PORT) {
            //Interface::get().stop();
            //Interface::get().reset();
            ClientKnowledgeBase::get().shutdown();
        }

        // On nettoie toutes les données
        std::cout << "Arret de la socket d'ecoute\n";
        mainSocket.close();

        std::cout << "Arret des threads de traitement (Attente du timeout)\n";
        for (unsigned int i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
        OperationQueue::get().clear();
        AnswerQueue::get().stop();
    } catch(std::exception& e) {
        std::cout << "Une erreur est survenue : " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Une erreur inconnue est survenue : " << std::endl;
    }

    return 0;
}
