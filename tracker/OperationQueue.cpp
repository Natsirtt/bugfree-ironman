#include "OperationQueue.hpp"


OperationQueue::OperationQueue() {
    // TODO construire les mutex
}

Operation OperationQueue::getNextOperation() {
    pthread_mutex_lock(&mReadSem);

    Operation op = mQueue.front();

    pthread_mutex_lock(&mModifyMutex);
    mQueue.pop();
    pthread_mutex_unlock(&mModifyMutex);

    return op;
}

void OperationQueue::addOperation(Operation op) {

    pthread_mutex_lock(&mModifyMutex);
    mQueue.push(op);

    pthread_mutex_unlock(&mReadSem);
    pthread_mutex_unlock(&mModifyMutex);
}
