#define _XOPEN_SOURCE
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include "SocketUDP.hpp"
#include "SocketUDPUtils.hpp"

#include <string>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512
#endif

SocketUDP::SocketUDP() {
	socket = socket(AF_INET, SOCK_DGRAM, 0);

	if (socket == -1) {
		perror("SocketUDP()");
		throw std::runtime_error("Could not create new socket");
	}

	local.name = "";
	local.ip = "";
	local.port = -1;
}

SocketUDP::SocketUDP(std::string adresse, int port) {
    SocketUDP();

	struct sockaddr_in in;
	if (initSockAddr(adresse, port, &in) == -1) {
		cout << "Error at SocketUDP(std::string adresse, int port)"
		throw std::runtime_error("Erreur d'initialisation de l'adresse"):
	}

	if (bind(soc->socket, (struct sockaddr*) &in, sizeof(struct sockaddr_in)) == -1) {
		perror("SocketUDP(std::string adresse, int port)");
		throw std::runtime_error("Erreur lors du bind de la socket");
	}

	local = getIdBySockAddr(&in);

	return soc;
}

std::string getLocalName() {
	std::string res = local.name;
	if ((res.size() == 0) && (socket != -1)) {
		res = getLocalIP(socket);
		if (res.size() != 0) {
			char buffer[BUFFER_SIZE];
			struct sockaddr_in in;
			memset(&in, 0, sizeof(struct sockaddr_in));
			in.sin_family = AF_INET;
			in.sin_addr.s_addr = inet_addr(res);
			int err = getnameinfo(
							(struct sockaddr*) &in, sizeof(struct sockaddr_in),
						   buffer, BUFFER_SIZE,
						   NULL, 0, NI_DGRAM);
			if (err == -1) {
				perror("getLocalName()");
				throw std::runtime_error("getnameinfo failed");
			}
			local.name = std::sring(buffer);
			res = local.name;
		}
	}
	return res;
}

std::string getLocalIP() {
	std::string res = local.ip;
	if ((res.size() == 0) && (socket != -1)) {
		struct sockaddr_in in;
		int size = sizeof(struct sockaddr_in);
		if (getpeername(socket->socket, (struct sockaddr*) &in, (socklen_t*)&size) == -1) {
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
		local.ip = std::string(buffer);
		res = local.ip;
	}
	return res;
}

int getLocalPort() {
	int port = local.port;

	if ((port == -1) && (socket->socket != -1)) {
		struct sockaddr_in in;
		int size = sizeof(struct sockaddr_in);
		if (getpeername(socket->socket, (struct sockaddr*) &in, (socklen_t*)&size) == -1) {
			perror("getLocalPort()");
			throw runtime_error("getpeername failed");
		}
		port = getPortByAddr(&in);
	}

	return port;
}

int write(const std::string adresse, int port, char* buffer, int length) {
	if (socket == -1) {
		fprintf(stderr, "Impossible d'envoyer des donnees sur une socket non ouverte\n");
		return -1;
	}
	struct sockaddr_in in;
	if (initSockAddr(adresse, port, &in) == -1) {
		fprintf(stderr, "Erreur d'initilisation de l'adresse\n");
		return -1;
	}
	int n = sendto(socket, buffer, length, 0,
					(struct sockaddr*) &in, sizeof(struct sockaddr_in));
	if (n == -1) {
		perror("writeToSocketUDP()");
		return -1;
	}
	return n;
}

int read(char* buffer, int length,
						std::string& adresse, int* port, int timeout) {
	if (socket == -1) {
		fprintf(stderr, "Impossible de recevoir des donnees sur une socket non ouverte\n");
		return -1;
	}
	fd_set set;
	FD_ZERO(&set);
	FD_SET(socket, &set);
	int err = 0;

	if (timeout > 0) {
		struct timeval time;
		time.tv_sec = timeout;
		time.tv_usec = 0;
		err = select(socket + 1, &set, NULL, NULL, &time);
	} else {
		err = select(socket + 1, &set, NULL, NULL, NULL);
	}
	if (err == -1) {
		perror("read()");
		return -1;
	} else if (err == 0) {
		return 0;
	}

	struct sockaddr_in in;
	int size = sizeof(struct sockaddr_in);
	int n = recvfrom(socket, buffer, length, 0,
					(struct sockaddr*) &in, (socklen_t*) &size);
	if (n == -1) {
		perror("read()");
		return -1;
	}
	if ((adresse.size() > 0) && (port != NULL)) {
		id i = getIdBySockAddr(&in);
		if (adresse.size() > 0) {
			adresse.swap(i.ip);
		}
		if (port != NULL) {
			*port = i.port;
		}
	}
	return n;
}

int close() {
	int err = 0;
	shutdown(socket, SHUT_RDWR);

	err = close(socket);

	return err;
}
