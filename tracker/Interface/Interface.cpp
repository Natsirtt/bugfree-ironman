#include "Interface.hpp"
#include "../ClientFile.hpp"
#include "../ClientKnowledgeBase.hpp"
#include "../Defines.hpp"

#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <cstdio>
#include <sys/select.h>
#include <cstdlib>
#include <termios.h>

Interface::Interface() : mCurrentPage(0) {
    mFiles = ClientKnowledgeBase::get().getFiles();
    draw();
}

Interface::~Interface() {
    reset();
}

int kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}
void* interface_thread(void* arg) {
    while (State::get().isRunning()) {
        //if (kbhit()) {
            //char c;
            //std::cin >> std::ws >> std::noskipws >> c;
            char c = getch();
            switch (c) {
            case 'p':
                Interface::get().precPage();
                break;
            case 's':
                Interface::get().nextPage();
                break;
            case 'q':
                //quick_exit(3);
                State::get().stop();
                kill(getpid(), SIGTERM);
                break;
            case 'u':
                Interface::get().update();
            default:
                break;
            }
        //}
        Interface::get().draw();
    }
    return NULL;
}

void Interface::start() {
    if (pthread_create(&mThread, NULL, interface_thread, NULL) == -1) {
        throw std::runtime_error("Impossible de lancer le thread d'interface\n");
    }
}

void Interface::stop() {
    if (pthread_join(mThread, NULL) == -1) {
        throw std::runtime_error("Impossible d'arreter le thread d'interface\n");
    }
}

void Interface::draw() {
    static int i = 0;
    std::stringstream line;

    //std::cout << "\033[2J"; // Efface l'écran
    std::cout << "\033[1;1H\033[1;4;40;37m"; // Retourne au debut puis change le mode d'écriture

    int begin = mCurrentPage * (ROW - 2);
    int end = begin + (ROW - 2);
    line << "Fichiers " << begin << " a " << std::min(begin + (ROW - 2), (int)mFiles.size()) << " " << i++ << " :";
    std::cout << line.str() << "\033[0;40;37m";
    std::cout << std::setw(COLUMN - line.str().length()) << std::left << "";

    std::cout << "\033[0;40;37m"; // Change le mode d'écriture
    int lineNb = 2;
    for (int i = begin; i < end; ++i) {
        line.str("");
        line.clear();
        if (i < (int)mFiles.size()) {
            line << mFiles[i]->getName() << " - " << mFiles[i]->getSize() << " octets " << mFiles[i]->getCompletion() << "%";
        } else {
            line << "";
        }
        std::cout << "\033[" << lineNb++ << ";1H";
        std::cout << std::setw(COLUMN) << std::left << line.str();
    }
    std::cout << "\033[" << ROW << ";1H";
    line.str("");
    line.clear();
    line << "quitter : q; page suivant : s; page precedante : p; mettre a jour : u";
    std::cout << std::setw(COLUMN) << std::left << line.str();
    std::cout.flush();
}

void Interface::precPage() {
    if (mCurrentPage > 0) {
        mCurrentPage--;
        draw();
    }
}

void Interface::nextPage() {
    if (mCurrentPage < (getMaxPageNumber() - 1)) {
        mCurrentPage++;
        draw();
    }
}

int Interface::getMaxPageNumber() {
    int nb = mFiles.size() / (ROW - 2);
    if ((mFiles.size() % (ROW - 2)) != 0) {
        nb++;
    }
    return nb;
}

void Interface::reset() {
    std::cout << "\033[1;1H\033[0m"; // Retourne au debut puis change le mode d'écriture
    std::cout << "\033[2J"; // Efface l'écran
}

void Interface::update() {
    mFiles = ClientKnowledgeBase::get().getFiles();
}

