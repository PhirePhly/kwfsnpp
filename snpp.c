// kwfsnppd - Simple Network Paging Protocol gateway to APRS-IS
// Copyright Kenneth Finnegan, 2014
//
//

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "kwfsnppd.h"
#include "snpp.h"
#include "util.h"

void snpp_listen () {
	int rc, i, yes = 1;
	fd_set snpp_sock;
	int snpp_fdmax;
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

	FD_ZERO(&snpp_sock);
	snpp_fdmax = -1;

	// Bind and listen on all available sockets for new connections
	for (nextif = svrif; nextif != NULL; nextif = nextif->ai_next) {
		int new_sock = socket(nextif->ai_family, nextif->ai_socktype,
				nextif->ai_protocol);
		if (new_sock == -1) {
			syslog(LOG_NOTICE, "Failed to create socket");
			continue;
		}
		
		rc = setsockopt(new_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if (rc == -1) {
			syslog(LOG_ERR, "setsockopt failed: %s", gai_strerror(rc));
			exit(EXIT_FAILURE);
		}

		// Remove IPv4 binding from v6 socket
		if (nextif->ai_family == AF_INET6) { 
			rc = setsockopt(new_sock, IPPROTO_IPV6, 
					IPV6_V6ONLY, &yes, sizeof(int));
			if (rc == -1) {
				syslog(LOG_ERR, "setsockopt failed: %s", gai_strerror(rc));
				exit(EXIT_FAILURE);
			}
		}


		rc = bind(new_sock, nextif->ai_addr, nextif->ai_addrlen);
		if (rc == -1) {
			close(new_sock);
			syslog(LOG_ERR, "Failed to bind socket: %s", gai_strerror(rc));
			continue;
		}

		rc = listen(new_sock, 10); // Backlog of 10
		if (rc == -1) {
			syslog(LOG_ERR, "socket listen failed: %s", gai_strerror(rc));
			exit(EXIT_FAILURE);
		}

		FD_SET(new_sock, &snpp_sock);
		if (snpp_fdmax < new_sock) {
			snpp_fdmax = new_sock;
		}
	}	

	if (snpp_fdmax < 0) {
		syslog(LOG_ERR, "Failed to bind to any socket");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(svrif);


	syslog(LOG_DEBUG, "Waiting for SNPP client connections");

	while (1) {
		fd_set snpp_newconn = snpp_sock;
		rc = select(snpp_fdmax + 1, &snpp_newconn, NULL, NULL, NULL);
		if (rc == -1) {
			syslog(LOG_ERR, "Select() failure in SNPP_listen()");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i <= snpp_fdmax; i++) {
			if (!FD_ISSET(i, &snpp_newconn))
				continue;

			struct sockaddr_storage client_addr;
			int sas_size = sizeof(client_addr);
			int client_fd;

			client_fd = accept(i, (struct sockaddr*)&client_addr, &sas_size);
			if (client_fd == -1) {
				syslog(LOG_ERR, "SNPP accept failure");
				continue;
			}


			// Spawn new thread to handle new connection
			pthread_t tclient;

			struct snpp_state *ptarg = (struct snpp_state*)
				malloc(sizeof(struct snpp_state));
			ptarg->fd = client_fd;
			memcpy(&(ptarg->addr), &client_addr, 
					sizeof(struct sockaddr_storage));

			pthread_create(&tclient, NULL, snpp_client, (void*) ptarg);
			pthread_detach(tclient);
		}
	}
}

static void * snpp_client(void *arg) {
	int rc;

	struct snpp_state snppstate;
	memcpy(&snppstate, arg, sizeof(struct snpp_state));
	free(arg);

	struct recvline_state rcvrbuf;
	memset(&rcvrbuf, 0, sizeof(struct recvline_state));
	rcvrbuf.fd = snppstate.fd;

	char buf[BUFFER_LEN];

	get_addr_str(buf, sizeof(buf), (struct sockaddr*) &(snppstate.addr));
	syslog(LOG_DEBUG, "New SNPP connection from %s", buf);

	snprintf(buf, sizeof(buf), "%s", SNPP_HELLO);
	rc = nsend(snppstate.fd, buf, strlen(buf));
	if (rc == -1) {
		goto cleanup;
	}

	while (1) {
		fd_set snpp_client;
		FD_ZERO(&snpp_client);
		FD_SET(snppstate.fd, &snpp_client);
		struct timeval timeout;
		timeout.tv_sec = svrstate.snpp_timeout;
		timeout.tv_usec = 0;
		rc = select(snppstate.fd+1, &snpp_client, NULL, NULL, &timeout);

		if (rc == 0) {
			snprintf(buf, sizeof(buf), "%s", SNPP_TIMEOUT);
			nsend(snppstate.fd, buf, strlen(buf));
			goto cleanup;
		}

		char *line;
		while (rc = recvline(&rcvrbuf, &line)) {
			if (rc == -1) goto cleanup;

			// Process each line from client
			printf("New line (%d): %s\n", rc, line);
		}
	}

cleanup:
	get_addr_str(buf, sizeof(buf), (struct sockaddr*) &(snppstate.addr));
	syslog(LOG_DEBUG, "Closing SNPP connection from %s", buf);

	close(snppstate.fd);
	return NULL;

}
