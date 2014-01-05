#include "Thread.hpp"
#include <iostream>
#include <stdexcept>
#include "OperationQueue.hpp"

void* traitement(void* arg);

Thread::Thread() {
    if (pthread_create(&mThread, NULL, traitement, NULL) != 0) {
        throw std::runtime_error("Erreur lors de la creation d'un thread");
    }
}

void Thread::join() {
    if (pthread_join(mThread, NULL) != 0) {
        throw std::runtime_error("Erreur lors de l'arret d'un thread");
    }
}


void* traitement(void* arg) {
    while (1) { // Faire une condition d'arret qui prenne en compte le join
        try {
            Operation op = OperationQueue::get().getNextOperation();

            op.getPacket()->exec(op.getAdresse());
        } catch(const std::exception& e) {
            std::cerr << "Une erreur est survenue dans un thread : " << e.what() << std::endl;
        }
    }
    return NULL;
}
