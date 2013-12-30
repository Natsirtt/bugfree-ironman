#include "Client.hpp"

#include <ctime>

Client::Client(std::string adresse) : mAdresse(adresse) {
    alive();
}

std::string Client::getAdresse() {
    return mAdresse;
}

void Client::alive() {
    mLastAlive = std::time(NULL);
}


bool Client::isAlive() {
    return (std::time(NULL) - mLastAlive) < CONNECTION_TIMEOUT;
}

void Client::updateFile(std::string filename, char* bitmap, int bitmapSize) {
    std::vector<char> v = mFiles[filename];
    v.clear();
    for (int i = 0; i < bitmapSize; ++i) {
        v.push_back(bitmap[i]);
    }
}

bool Client::hasFile(std::string filename) {
    return mFiles[filename].size() > 0;
}

bool isNthBitSet(char c, int n) {
    static unsigned char mask[] = {128, 64, 32, 16, 8, 4, 2, 1};
    return ((c & mask[n]) != 0);
}

bool Client::hasPartition(std::string filename, int partNb) {
    std::vector<char> v = mFiles[filename];
    return (v.size() >= (partNb / 8)) && isNthBitSet(v[partNb / 8], partNb % 8);
}
