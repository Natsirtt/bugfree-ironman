#ifndef ANSWERQUEUE_HPP
#define ANSWERQUEUE_HPP

#include <queue>
#include <pthread.h>

#include "SocketUDP.hpp"
#include "Packets/IPacket.hpp"

/**
 * Classe représentant une file de réponses à envoyer en utilisation un thread séparé.
 * Note : ThreadSafe
 */
class AnswerQueue {
    public:
        static AnswerQueue& get() {
            static AnswerQueue aq;
            return aq;
        }

        /**
         * Démarre le thread de traitement et d'envoie des réponses.
         */
        void start();
        /**
         * Arrete le thread de traitement et d'envoie des réponses.
         */
        void stop();

        void send(IPacket* packet, std::string& adresse, int port);
        void sendToClient(IPacket* packet, std::string& adresse);
        void sendToTracker(IPacket* packet, std::string& adresse);

        struct Answer {
            IPacket* packet;
            std::string adresse;
            int port;
        };

        Answer getNextAnswer();

    private:
        AnswerQueue();
        ~AnswerQueue();

        std::queue<Answer> mAnswers;

        pthread_t mThread;
        pthread_mutex_t mModifyMutex;
        int mReadSem;
};

#endif
