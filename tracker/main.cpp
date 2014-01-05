#include <iostream>
#include <sstream>

 #include "File.hpp"
 #include "KnowledgeBase.hpp"
 #include "SocketUDP.hpp"
 #include "Operation.hpp"
 #include "NetworkTranslator.hpp"
 #include "OperationQueue.hpp"
 #include "Thread.hpp"
 #include "Defines.hpp"
 #include "AnswerQueue.hpp"
 #include <cstring>

using namespace std;

void help() {
    std::cout << "Erreur d'utilisation." << std::endl;
    std::cout << "  ./prog CLIENT|TRACKER [ip_tracker] [nb_threads]" << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        try {
            bool client = false;
            std::string trackerIP;
            int nbThreads = THREAD_NUMBER;

            if ((argc > 1) && (strcmp(argv[1], "CLIENT") == 0)) {
                client = true;
                if (argc > 2) {
                    trackerIP = std::string(argv[2]);
                    if (argc > 3) {
                        std::stringstream ss(argv[3]);
                        ss >> nbThreads;
                    }
                }
            } else if ((argc > 1) && (strcmp(argv[1], "TRACKER") == 0)) {
                client = false;
            } else if ((argc > 1) && (strcmp(argv[1], "TRACKER") == 0)) {
                help();
            }
        } catch (...) {
            help();
        }


        // Un appel � la file d'op�rations (l'oblige � se constuire si pas encore fait)
        OperationQueue::get();

        // Un appel � la file des r�ponses (l'oblige � se constuire si pas encore fait) et d�marre de thread
        AnswerQueue::get();
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

            // On r�cup�re un paquet � partir de la socket
            packet = nt.readPacket(adresse, &port, 0);
            if (packet == NULL) {
                continue;
            }

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
        AnswerQueue::get().stop();
    } catch(std::exception& e) {
        std::cerr << "Une erreur est survenue : " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Une erreur inconnue est survenue : " << std::endl;
    }

    return 0;
}
