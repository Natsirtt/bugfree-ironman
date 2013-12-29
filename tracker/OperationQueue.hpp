#ifndef OPERATION_QUEUE_HPP
#define OPERATION_QUEUE_HPP

#include <queue>
#include "Operation.hpp"

/**
 * Singleton repr�sentant une file d'op�rations.
 * Note : Toutes les op�rations sont threadSafe.
 */
class OperationQueue {

    public:
        static OperationQueue& get() {
            static OperationQueue opq;
            return opq;
        }

        Operation getNextOperation();

        void addOperation(Operation op);

        void clear();

    private:
        OperationQueue();

        std::queue<Operation> mQueue;
        // Mutex permettant de savoir si on peut lire dans la file
        int mReadSem;
        // Mutex permettant de savoir si on peut mofier la file
        pthread_mutex_t mModifyMutex;
};

#endif // OPERATION_QUEUE_HPP