#ifndef OPERATION_QUEUE_HPP
#define OPERATION_QUEUE_HPP

#include <queue>
#include "Operation.hpp"

/**
 * Singleton représentant une file d'opérations.
 */
class OperationQueue {

    public:
        static OperationQueue& get() {
            static OperationQueue opq;
            return opq;
        }

        Operation getNextOperation();

        void addOperation(Operation op);

    private:
        OperationQueue();

        std::queue<Operation> mQueue;
        // Mutex permettant de savoir si on peut lire dans la file
        pthread_mutex_t mReadSem;
        // Mutex permettant de savoir si on peut mofier la file
        pthread_mutex_t mModifyMutex;
};

#endif // OPERATION_QUEUE_HPP
