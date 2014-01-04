#include "OperationQueue.hpp"

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdexcept>
#include <sys/sem.h>
#include <cstdio>

#define KEY_FILE "opqueue"
#define KEY_ID 1


OperationQueue::OperationQueue() {
    // Initialisation du semaphore
    key_t key = ftok(KEY_FILE, KEY_ID);
    if (key == -1) {
        throw std::runtime_error("Impossible de trouver la cle du semaphore");
    }

    mReadSem = semget(key, 1, IPC_CREAT | 0666);
    if (mReadSem == -1) {
        throw std::runtime_error("Impossible de generer un semaphore");
    }
    if (semctl(mReadSem, 0, SETVAL, 0) == -1) {
        throw std::runtime_error("Erreur lors de l'initialisation d'un semaphore");
    }

    // Initialisation du mutex
    pthread_mutex_init (&mModifyMutex, NULL);
}

Operation OperationQueue::getNextOperation() {
    sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = 0;
    if (semop(mReadSem, &sop, 1) == -1) {
        perror("semop");
        throw std::runtime_error("Erreur lors de la decrementation d'un semaphore");
    }

    Operation op = mQueue.front();

    pthread_mutex_lock(&mModifyMutex);
    mQueue.pop();
    pthread_mutex_unlock(&mModifyMutex);

    return op;
}

void OperationQueue::addOperation(Operation op) {
    pthread_mutex_lock(&mModifyMutex);
    mQueue.push(op);

    sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;
    semop(mReadSem, &sop, 1);
    pthread_mutex_unlock(&mModifyMutex);
}


void OperationQueue::clear() {
    pthread_mutex_lock(&mModifyMutex);

    while (!mQueue.empty()) {
        Operation op = mQueue.front();
        delete op.getPacket();
        mQueue.pop();
    }

    semctl(mReadSem, 1, SETVAL, 0);
    pthread_mutex_unlock(&mModifyMutex);
}
