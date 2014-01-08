#define _XOPEN_SOURCE

#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include "SocketUDP.hpp"

#include <string>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512
#endif

SocketUDP::SocketUDP() {
	mSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (mSocket == -1) {
		perror("SocketUDP()");
		throw std::runtime_error("Could not create new socket");
	}

	mLocal.name = "";
	mLocal.ip = "";
	mLocal.port = -1;
}

SocketUDP::SocketUDP(std::string adresse, int port) {
	mSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (mSocket == -1) {
		perror("SocketUDP()");
		throw std::runtime_error("Could not create new socket");
	}

	mLocal.name = "";
	mLocal.ip = "";
	mLocal.port = -1;

	struct sockaddr_in in;
	if (initSockAddr(adresse, port, &in) == -1) {
		std::cerr << "Error at SocketUDP(std::string adresse, int port)\n";
		throw std::runtime_error("Erreur d'initialisation de l'adresse");
	}

	if (bind(mSocket, (struct sockaddr*) &in, sizeof(struct sockaddr_in)) == -1) {
		perror("SocketUDP(std::string adresse, int port)");
		std::cout << mSocket << std::endl;
		throw std::runtime_error("Erreur lors du bind de la socket");
	}

	mLocal = getIdBySockAddr(&in);
}

std::string SocketUDP::getLocalName() {
	std::string res = mLocal.name;
	if ((res.size() == 0) && (mSocket != -1)) {
		res = getLocalIp();
		if (res.size() != 0) {
			char buffer[BUFFER_SIZE];
			struct sockaddr_in in;
			memset(&in, 0, sizeof(struct sockaddr_in));
			in.sin_family = AF_INET;
			in.sin_addr.s_addr = inet_addr(res.c_str());
			int err = getnameinfo(
							(struct sockaddr*) &in, sizeof(struct sockaddr_in),
						   buffer, BUFFER_SIZE,
						   NULL, 0, NI_DGRAM);
			if (err == -1) {
				perror("getLocalName()");
				throw std::runtime_error("getnameinfo failed");
			}
			mLocal.name = std::string(buffer);
			res = mLocal.name;
		}
	}
	return res;
}

std::string SocketUDP::getLocalIp() {
	std::string res = mLocal.ip;
	if ((res.size() == 0) && (mSocket != -1)) {
		struct sockaddr_in in;
		int size = sizeof(struct sockaddr_in);
		if (getpeername(mSocket, (struct sockaddr*) &in, (socklen_t*)&size) == -1) {
			perror("getLocalIp()");
			throw std::runtime_error("getpeername failed");
		}
		char buffer[BUFFER_SIZE];
		int err = getnameinfo(
						(struct sockaddr*) &in, sizeof(struct sockaddr_in),
					   buffer, BUFFER_SIZE,
					   NULL, 0, NI_DGRAM | NI_NUMERICHOST);
		if (err == -1) {
			perror("getLocalIp()");
			throw std::runtime_error("getnameinfo failed");
		}
		mLocal.ip = std::string(buffer);
		res = mLocal.ip;
	}
	return res;
}

int SocketUDP::getLocalPort() {
	int port = mLocal.port;

	if ((port == -1) && (mSocket != -1)) {
		struct sockaddr_in in;
		int size = sizeof(struct sockaddr_in);
		if (getpeername(mSocket, (struct sockaddr*) &in, (socklen_t*)&size) == -1) {
			perror("getLocalPort()");
			throw std::runtime_error("getpeername failed");
		}
		port = getPortByAddr(&in);
		mLocal.port = port;
	}

	return port;
}

int SocketUDP::write(const std::string adresse, int port, char* buffer, int length) {
	if (mSocket == -1) {
		std::cerr << "Impossible d'envoyer des donnees sur une socket non ouverte\n";
		return -1;
	}
	struct sockaddr_in in;
	if (initSockAddr(adresse, port, &in) == -1) {
		std::cerr << "Erreur d'initilisation de l'adresse d'envoie des donnees\n";
		return -1;
	}
	int n = sendto(mSocket, buffer, length, 0,
					(struct sockaddr*) &in, sizeof(struct sockaddr_in));
	if (n == -1) {
		perror("writeToSocketUDP()");
		return -1;
	}
	return n;
}

int SocketUDP::read(char* buffer, int length,
						std::string& adresse, int* port, int timeout) {
	if (mSocket == -1) {
		std::cerr << "Impossible de recevoir des donnees sur une socket non ouverte\n";
		return -1;
	}
	fd_set set;
	FD_ZERO(&set);
	FD_SET(mSocket, &set);
	int err = 0;

	if (timeout > 0) {
		struct timeval time;
		time.tv_sec = timeout;
		time.tv_usec = 0;
		err = select(mSocket + 1, &set, NULL, NULL, &time);
	} else {
		err = select(mSocket + 1, &set, NULL, NULL, NULL);
	}
	if (err == -1) {
		perror("read()");
		return -1;
	} else if (err == 0) {
		return 0;
	}

	struct sockaddr_in in;
	int size = sizeof(struct sockaddr_in);
	int n = recvfrom(mSocket, buffer, length, 0,
					(struct sockaddr*) &in, (socklen_t*) &size);
	if (n == -1) {
		perror("read()");
		return -1;
	}
	if ((adresse.size() <= 0) || (port != NULL)) {
		Id i = getIdBySockAddr(&in);
		if (adresse.size() <= 0) {
			adresse.swap(i.ip);
		}
		if (port != NULL) {
			*port = i.port;
		}
	}
	return n;
}

int SocketUDP::close() {
	int err = 0;
	shutdown(mSocket, SHUT_RDWR);

	err = ::close(mSocket);
	mSocket = -1;

	return err;
}

int SocketUDP::initSockAddr(std::string adresse, int port, struct sockaddr_in* in) {
    memset(in, 0, sizeof(struct sockaddr_in));
    struct addrinfo hint;
    memset(&hint, 0, sizeof (struct addrinfo));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    struct addrinfo* res = NULL;
    int s;
    char service[10];
    sprintf(service, "%d", port);
    const char* addr = NULL;
    if (adresse.size() > 0) {
        addr = adresse.c_str();
    }
    if ((s = getaddrinfo(addr, service, &hint, &res)) == -1) {
        std::cout << "getaddrinfo: " << gai_strerror(s) << std::endl;
        return -1;
    }
    if (res == NULL) {
        std::cout << "getaddrinfo: " << gai_strerror(s) << std::endl;
        return -1;
    }
    memcpy(in, res->ai_addr, std::min(sizeof(struct sockaddr_in), (long unsigned int)res->ai_addrlen));
    freeaddrinfo(res);
    return 0;
}

SocketUDP::Id SocketUDP::getIdBySockAddr(const struct sockaddr_in* in) {
    Id i = {std::string(), std::string(), -1};

    char ip[BUFFER_SIZE];
    char port[BUFFER_SIZE];
    getnameinfo((struct sockaddr*) in, (socklen_t) sizeof (struct sockaddr_in),
            ip, BUFFER_SIZE,
            port, BUFFER_SIZE,
            NI_NUMERICHOST | NI_NUMERICSERV);
    i.ip = std::string(ip);
    i.port = strtol(port, NULL, 0);

    return i;
}

int SocketUDP::getPortByAddr(const struct sockaddr_in* in) {
    char buff[BUFFER_SIZE];
    int err = getnameinfo(
            (struct sockaddr*) in, (socklen_t) sizeof (struct sockaddr_in),
            NULL, 0,
            buff, BUFFER_SIZE,
            NI_DGRAM | NI_NUMERICSERV);
    if (err == -1) {
        perror("getnameinfo");
        return -1;
    }
    int port = strtol(buff, NULL, 0);
    return port;
}

