#ifndef OPERATION_HPP
#define OPERATION_HPP

#include <string>
#include "Packets/IPacket.hpp"

class Operation {

    public:
        Operation(IPacket* packet, std::string adresse, int port);

        std::string getAdresse();
        int getPort();
        IPacket* getPacket();

    private:
        IPacket* mPacket;
        std::string mAdresse;
        int mPort;
};

#endif // ICOMMAND_HPP
