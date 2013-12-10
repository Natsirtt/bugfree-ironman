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
        SocketUDP(const char* adresse, int port);
        ~SocketUDP();


        std::string getLocalName();
        std::string getLocalIP();
        int getLocalPort();
        int write(const std::string adresse, int port, char* buffer, int length);
		/**
        * 'adresse' doit etre initialisé et contenir 15 octets.
        */
        int read(char* buffer, int length,
						std::string& adresse, int* port, int timeout);

        int close();

	protected:

	private:
        struct id {
            std::string name;
            std::string ip;
            int port;
        };

        int socket;
        id local;
};

#endif
