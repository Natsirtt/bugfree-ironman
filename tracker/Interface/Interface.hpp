#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <vector>
#include <pthread.h>

class ClientFile;

#define ROW 24
#define COLUMN 80

class Interface {
    public:
        static Interface& get() {
            static Interface interface;
            return interface;
        }

        void start();
        void stop();

        void draw();

        void precPage();
        void nextPage();

        int getMaxPageNumber();

    private:
        Interface();

        pthread_t mThread;

        int mCurrentPage;
        std::vector<ClientFile*> mFiles;
};

#endif // INTERFACE_HPP
