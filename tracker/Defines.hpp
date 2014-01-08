#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <csignal>
#include <iostream>

// Le nombre de threads de traitement qu'on utilise
#define THREAD_NUMBER 10

#define TRACKER_PORT 9876
#define CLIENT_PORT 9875

//La taille d'un bloc
#define BLOCK_SIZE 512ll
#define BLOCK_PER_PARTITION 2048ll
#define PARTITION_SIZE BLOCK_SIZE * BLOCK_PER_PARTITION

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
            //std::cout << "stop" << std::endl;
            mRunning = false;
        }

        void setTrackerIp(std::string ip) {
            mTrackerIp = ip;
        }

        std::string getTrackerIp() {
            return mTrackerIp;
        }


    private:
        static void handler_function (int parameter) {
            if (parameter != SIGALRM) {
                std::cout << "signal " << parameter << std::endl;
                State::get().stop();
            }
        }

        State() : mRunning(true) {
            int signals[] = {
                SIGABRT,
                SIGFPE,
                SIGILL,
                SIGINT,
                SIGTERM,
                SIGALRM
            };
            for (unsigned int i = 0; i < sizeof(signals) / sizeof(int); ++i) {
                signal (signals[i], handler_function);
            }
        }

        bool mRunning;
        std::string mTrackerIp;
};

#endif // DEFINES_HPP
