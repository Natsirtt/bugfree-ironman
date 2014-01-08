#include "AnswerQueue.hpp"
#include "Defines.hpp"

#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdexcept>
#include <sys/sem.h>
#include <cstdio>
#include <errno.h>

#define KEY_FILE "opqueue"
#define KEY_ID 2

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
    struct seminfo* __buf;
};

AnswerQueue::AnswerQueue() : mThread(-1), mRunning(false)  {
    // Initialisation du semaphore
    key_t key = ftok(KEY_FILE, KEY_ID);
    if (key == -1) {
        throw std::runtime_error("Impossible de trouver la cle du semaphore");
    }

    mReadSem = semget(key, 1, IPC_CREAT | 0666);
    if (mReadSem == -1) {
        throw std::runtime_error("Impossible de generer un semaphore");
    }

    semun sAmoi;
    sAmoi.val = 0;
    if (semctl(mReadSem, 0, SETVAL, sAmoi) == -1) {
        throw std::runtime_error("Erreur lors de l'initialisation d'un semaphore");
    }

    // Initialisation du mutex
    if (pthread_mutex_init(&mModifyMutex, NULL) == -1) {
        perror("pthread_mutex_init");
        throw std::runtime_error("Erreur lors de l'initialisation d'un mutex");
    }
}

AnswerQueue::~AnswerQueue() {
    stop();
    pthread_mutex_destroy(&mModifyMutex);

    semctl(mReadSem, 0, IPC_RMID);
}

void* answer_thread(void* arg) {
    SocketUDP answerSocket;

    while (State::get().isRunning()) {
        try {
            AnswerQueue::Answer a = AnswerQueue::get().getNextAnswer();
            answerSocket.write(a.adresse, a.port, a.packet->toData(), a.packet->getSize());
        } catch(const std::logic_error& e) {
            // RIEN
        } catch(const std::exception& e) {
            std::cout << "Une erreur est survenue dans un thread : " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Une erreur est survenue dans un thread" << std::endl;
        }
    }
    return NULL;
}


void AnswerQueue::start() {
    if (!mRunning) {
        if (pthread_create(&mThread, NULL, answer_thread, NULL) != 0) {
            throw std::runtime_error("Erreur lors de la creation d'un thread");
        }
        mRunning = true;
    }
}

void AnswerQueue::stop() {
    if (mRunning) {
        if (pthread_join(mThread, NULL) != 0) {
            throw std::runtime_error("Erreur lors de l'arret d'un thread");
        }
        mRunning = false;
    }
}

AnswerQueue::Answer AnswerQueue::getNextAnswer() {
    sembuf sop = {0, -1, 0};
    struct timespec timeout;
    timeout.tv_sec = 10;
    timeout.tv_nsec = 0;
    if (semtimedop(mReadSem, &sop, 1, &timeout) == -1) {
        if (errno == EAGAIN) {
            throw std::logic_error("Timeout");
        }
        perror("semop");
        throw std::runtime_error("Erreur lors de la decrementation d'un semaphore");
    }

    Answer ans = mAnswers.front();

    if (pthread_mutex_lock(&mModifyMutex) == -1) {
        perror("pthread_mutex_lock");
        throw std::runtime_error("Erreur lors du lock d'un mutex");
    }
    mAnswers.pop();
    if (pthread_mutex_unlock(&mModifyMutex) == -1) {
        perror("pthread_mutex_unlock");
        throw std::runtime_error("Erreur lors du unlock d'un mutex");
    }

    return ans;
}

void AnswerQueue::send(IPacket* packet, std::string& adresse, int port) {
    Answer a;
    a.packet = packet;
    a.adresse = adresse;
    a.port = port;

    if (pthread_mutex_lock(&mModifyMutex) == -1) {
        perror("pthread_mutex_lock");
        throw std::runtime_error("Erreur lors du lock d'un mutex");
    }

    mAnswers.push(a);

    sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;
    if (semop(mReadSem, &sop, 1) == -1) {
        perror("semop");
        throw std::runtime_error("Erreur lors de l'incrementation d'un semaphore");
    }

    if (pthread_mutex_unlock(&mModifyMutex) == -1) {
        perror("pthread_mutex_unlock");
        throw std::runtime_error("Erreur lors du unlock d'un mutex");
    }

}

void AnswerQueue::sendToClient(IPacket* packet, std::string& adresse) {
    send(packet, adresse, CLIENT_PORT);
}

void AnswerQueue::sendToTracker(IPacket* packet, std::string& adresse) {
    send(packet, adresse, TRACKER_PORT);
}

