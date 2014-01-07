#include "OperationQueue.hpp"

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdexcept>
#include <sys/sem.h>
#include <cstdio>
#include <errno.h>
#include <stdexcept>

#define KEY_FILE "opqueue"
#define KEY_ID 1


OperationQueue::OperationQueue() {
    // Initialisation du semaphore
    key_t key = ftok(KEY_FILE, KEY_ID);
    if (key == -1) {
        perror("ftok");
        throw std::runtime_error("Impossible de trouver la cle du semaphore");
    }

    mReadSem = semget(key, 1, IPC_CREAT | 0666);
    if (mReadSem == -1) {
        perror("semget");
        throw std::runtime_error("Impossible de generer un semaphore");
    }
    if (semctl(mReadSem, 0, SETVAL, 0) == -1) {
        perror("semctl");
        throw std::runtime_error("Erreur lors de l'initialisation d'un semaphore");
    }

    // Initialisation du mutex
    if (pthread_mutex_init (&mModifyMutex, NULL) == -1) {
        perror("pthread_mutex_init");
        throw std::runtime_error("Erreur lors de l'initialisation d'un mutex");
    }
}

Operation OperationQueue::getNextOperation() {
    sembuf sop = {0, -1, 0};
    struct timespec timeout = {10, 0};
    if (semtimedop(mReadSem, &sop, 1, &timeout) == -1) {
        if (errno == EAGAIN) {
            throw std::logic_error("Timeout");
        }
        perror("semop");
        throw std::runtime_error("Erreur lors de la decrementation d'un semaphore");
    }

    Operation op = mQueue.front();

    if (pthread_mutex_lock(&mModifyMutex) == -1) {
        perror("pthread_mutex_lock");
        throw std::runtime_error("Erreur lors du lock d'un mutex");
    }
    mQueue.pop();
    if (pthread_mutex_unlock(&mModifyMutex) == -1) {
        perror("pthread_mutex_unlock");
        throw std::runtime_error("Erreur lors du unlock d'un mutex");
    }

    return op;
}

void OperationQueue::addOperation(Operation op) {
    if (pthread_mutex_lock(&mModifyMutex) == -1) {
        perror("pthread_mutex_lock");
        throw std::runtime_error("Erreur lors du lock d'un mutex");
    }
    mQueue.push(op);

    sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;
    semop(mReadSem, &sop, 1);
    if (pthread_mutex_unlock(&mModifyMutex) == -1) {
        perror("pthread_mutex_unlock");
        throw std::runtime_error("Erreur lors du unlock d'un mutex");
    }
}


void OperationQueue::clear() {
    if (pthread_mutex_lock(&mModifyMutex) == -1) {
        perror("pthread_mutex_lock");
        throw std::runtime_error("Erreur lors du lock d'un mutex");
    }

    while (!mQueue.empty()) {
        Operation op = mQueue.front();
        delete op.getPacket();
        mQueue.pop();
    }

    if (semctl(mReadSem, 0, SETVAL, 0) == -1) {
        throw std::runtime_error("Erreur lors de la remise a 0 d'un semaphore");
    }
    if (pthread_mutex_unlock(&mModifyMutex) == -1) {
        perror("pthread_mutex_unlock");
        throw std::runtime_error("Erreur lors du unlock d'un mutex");
    }
}
