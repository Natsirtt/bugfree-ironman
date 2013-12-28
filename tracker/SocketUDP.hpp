#ifndef SOCKETUDP_HPP
#define SOCKETUDP_HPP

#include <string>

class SocketUDP
{
	public:
		SocketUDP();
        /**
        * Créé une socket attachée
        */
        SocketUDP(std::string adresse, int port);

        std::string getLocalName();
        std::string getLocalIp();
        int getLocalPort();
        int write(const std::string adresse, int port, char* buffer, int length);

        int read(char* buffer, int length,
						std::string& adresse, int* port, int timeout);

        int close();

	protected:

    private:
        struct Id {
            std::string name;
            std::string ip;
            int port;
        };
        int initSockAddr(std::string adresse, int port, struct sockaddr_in* in);
        Id getIdBySockAddr(const struct sockaddr_in* in);
        int getPortByAddr(const struct sockaddr_in* in);

        int mSocket;
        Id mLocal;
};

#endif
