#include "AnswerQueue.hpp"
#include "Defines.hpp"

#include <stdexcept>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdexcept>
#include <sys/sem.h>

#define KEY_FILE "opqueue"
#define KEY_ID 2

AnswerQueue::AnswerQueue() : mThread(-1)  {
    // Initialisation du semaphore
    key_t key = ftok(KEY_FILE, KEY_ID);
    if (key == -1) {
        throw std::runtime_error("Impossible de trouver la cle du semaphore");
    }

    mReadSem = semget(key, 1, IPC_CREAT | 0666);
    if (mReadSem == -1) {
        throw std::runtime_error("Impossible de generer un semaphore");
    }
    semctl(mReadSem, 1, SETVAL, 0);

    // Initialisation du mutex
    pthread_mutex_init (&mModifyMutex, NULL);

}

AnswerQueue::~AnswerQueue() {
    // TODO des-initialiser les mutex, etc...
}

void* answer_thread(void* arg) {
    SocketUDP answerSocket;

    while (1) { // TODO faire une condition d'arret avec join
        AnswerQueue::Answer a = AnswerQueue::get().getNextAnswer();
        answerSocket.write(a.adresse, a.port, a.packet->toData(), a.packet->getSize());
    }
    return NULL;
}


void AnswerQueue::start() {
    if (pthread_create(&mThread, NULL, answer_thread, NULL) != 0) {
        throw std::runtime_error("Erreur lors de la creation d'un thread");
    }
}

void AnswerQueue::stop() {
    if (pthread_join(mThread, NULL) != 0) {
        throw std::runtime_error("Erreur lors de l'arret d'un thread");
    }
}

AnswerQueue::Answer AnswerQueue::getNextAnswer() {
    sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = 0;
    semop(mReadSem, &sop, 1);

    Answer ans = mAnswers.front();

    pthread_mutex_lock(&mModifyMutex);
    mAnswers.pop();
    pthread_mutex_unlock(&mModifyMutex);

    return ans;
}

void AnswerQueue::send(IPacket* packet, std::string& adresse, int port) {
    Answer a;
    a.packet = packet;
    a.adresse = adresse;
    a.port = port;

    pthread_mutex_lock(&mModifyMutex);

    mAnswers.push(a);

    sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;
    semop(mReadSem, &sop, 1);
    pthread_mutex_unlock(&mModifyMutex);
}

void AnswerQueue::sendToClient(IPacket* packet, std::string& adresse) {
    send(packet, adresse, CLIENT_PORT);
}

void AnswerQueue::sendToTracker(IPacket* packet, std::string& adresse) {
    send(packet, adresse, TRACKER_PORT);
}

