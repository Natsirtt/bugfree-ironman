#include "Interface.hpp"
#include "../ClientFile.hpp"
#include "../ClientKnowledgeBase.hpp"

#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <sstream>

Interface::Interface() : mCurrentPage(0) {
    mFiles = ClientKnowledgeBase::get().getFiles();
}

void* interface_thread(void* arg) {
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
    std::stringstream line;

    std::cout << "\033[2J"; // Efface l'écran
    std::cout << "\033[1;1H\033[1;4;40;37m"; // Retourne au debut puis change le mode d'écriture

    int begin = mCurrentPage * (ROW - 2);
    int end = begin + (ROW - 2);
    line << "Fichiers " << begin << " a " << std::min(begin + (ROW - 2), (int)mFiles.size()) << " :\033[0;40;37m";
    std::cout << std::setw(COLUMN) << std::left << line.str();

    std::cout << "\033[0;40;37m"; // Change le mode d'écriture
    for (int i = begin; i < end; ++i) {
        line.str("");
        line.clear();
        if (i < (int)mFiles.size()) {
            line << "\n" << mFiles[i]->getName();
        } else {
            line << "\n";
        }

        std::cout << std::setw(COLUMN) << std::left << line.str();
    }
    std::cout << "\033[" << ROW << ";1H";
    line.str("");
    line.clear();
    line << "quitter : q; page suivant : s; page precedante : p";
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
    if (mCurrentPage < getMaxPageNumber()) {
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
