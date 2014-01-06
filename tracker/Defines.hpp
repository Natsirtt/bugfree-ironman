#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <csignal>
#include <iostream>

// Le nombre de threads de traitement qu'on utilise
#define THREAD_NUMBER 5

#define TRACKER_PORT 98765
#define CLIENT_PORT 98764

//La taille d'un bloc
#define BLOCK_SIZE 4096
#define PARTITION_SIZE BLOCK_SIZE * 256

//Le répertoire de téléchargement des fichiers
#define FILES_PATH "files/"


class State {
    public:
        static State& get() {
            static State state;
            return state;
        }

        bool isRunning() {
            return mRunning;
        }

        void stop() {
            mRunning = false;
        }

    private:
        static void handler_function (int parameter) {
            State::get().stop();
        }

        State() : mRunning(true) {
            int signals[] = {
                SIGABRT,
                SIGFPE,
                SIGILL,
                SIGINT,
                SIGSEGV,
                SIGTERM
            };
            for (unsigned int i = 0; i < sizeof(signals) / sizeof(int); ++i) {
                signal (signals[i], handler_function);
            }
        }

        bool mRunning;
};


#endif // DEFINES_HPP
