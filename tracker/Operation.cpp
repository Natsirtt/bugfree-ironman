#include "Operation.hpp"

Operation::Operation(IPacket* packet, std::string adresse, int port) : mPacket(packet), mAdresse(adresse), mPort(port) {

}

std::string Operation::getAdresse() {
    return mAdresse;
}

int Operation::getPort() {
    return mPort;
}

IPacket* Operation::getPacket() {
    return mPacket;
}

