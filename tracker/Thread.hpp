#ifndef THREAD_HPP
#define THREAD_HPP

#include <pthread.h>

class Thread {
    public:

        Thread();
        void join();

    private:
        pthread_t mThread;
};

#endif // THREAD_HPP
