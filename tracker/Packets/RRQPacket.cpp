#include "RRQPacket.hpp"
#include "../Opcode.hpp"

RRQPacket::RRQPacket(std::string filename, int partition) : mFileName(filename), mPartition(partition) {
}

RRQPacket::RRQPacket(char* data) {
    // TODO
}

RRQPacket::~RRQPacket() {

}

int RRQPacket::getOpcode() {
    return RRQ;
}

int RRQPacket::getSize() {
    return 0; // TODO
}

char* RRQPacket::toData() {
    // TODO
    return NULL;
}

void RRQPacket::exec(std::string adresse) {
    // TODO
}
