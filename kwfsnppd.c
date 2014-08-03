// kwfsnpp - Simple Network Paging Protocol gateway to APRS-IS
// Copyright Kenneth Finnegan, 2014
//
//

#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "kwfsnppd.h"
#include "util.h"

int main (int argc, char **argv) {
	int rc;

	openlog(NULL, LOG_PERROR | LOG_PID, LOG_DAEMON);

	// Defaults
	svrstate.snpp_port = "444";

	while (1) {
		rc = getopt(argc, argv, "p:h");
		if (rc < 0)
			break;

		switch (rc) {
			case 'p':
				svrstate.snpp_port = optarg;
				break;
			case 'h':
			default:
				goto usage;
		}
	}

	// spawn APRS-IS helper thread
	// TODO

	// Block on SNPP service and create new SNPP client threads ad-hoc
	snpp_listen();

	return 0;

usage:
	fprintf(stderr, "Syntax: %s [-h]\n", argv[0]);
	exit(1);

}


void snpp_listen () {
	int rc, yes = 1;
	int snpp_sock;
	struct addrinfo hints, *svrif, *nextif;

	syslog(LOG_DEBUG, "Binding to SNPP port %s", svrstate.snpp_port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	rc = getaddrinfo(NULL, svrstate.snpp_port, &hints, &svrif);
	if (rc) {
		syslog(LOG_ERR, "getaddrinfo failed: %s", gai_strerror(rc));
		exit(EXIT_FAILURE);
	}

	// TODO: Bind to all available sockets...
	for (nextif = svrif; nextif != NULL; nextif = nextif->ai_next) {
		snpp_sock = socket(nextif->ai_family, nextif->ai_socktype,
				nextif->ai_protocol);
		if (snpp_sock == -1) {
			syslog(LOG_ERR, "Failed to create socket");
			continue;
		}
		
		rc = setsockopt(snpp_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if (rc == -1) {
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}

		rc = bind(snpp_sock, nextif->ai_addr, nextif->ai_addrlen);
		if (rc == -1) {
			close(snpp_sock);
			syslog(LOG_ERR, "Failed to bind to socket");
			continue;
		}

		break;
	}	

	if (nextif == NULL) {
		syslog(LOG_ERR, "Failed to bind to any socket");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(svrif);

	rc = listen(snpp_sock, 10); // Backlog of 10
	if (rc == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	syslog(LOG_DEBUG, "Waiting for SNPP client connections");

	while (1) {
		struct sockaddr_storage client_addr;
		int sin_size = sizeof(client_addr);
		int client_fd;

		client_fd = accept(snpp_sock, (struct sockaddr*)&client_addr, &sin_size);
		if (client_fd == -1) {
			syslog(LOG_ERR, "SNPP accept failure");
			continue;
		}

		char buf[INET6_ADDRSTRLEN];

		inet_ntop(client_addr.ss_family, 
				get_in_addr((struct sockaddr*)&client_addr),
					buf, sizeof(buf));

		syslog(LOG_DEBUG, "New SNPP connection from %s", buf);

		rc = send(client_fd, "421 SNPP (V0) Gateway Ready\r\n", 29, 0);
		if (rc == -1) {
			perror("send");
			close(client_fd);
			exit(EXIT_FAILURE);
		}

		close(client_fd);
	}
}
