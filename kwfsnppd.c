// kwfsnppd - Simple Network Paging Protocol gateway to APRS-IS
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
	svrstate.snpp_timeout = 600;
	svrstate.mess_queue = "/tmp/kwfsnppd";

	while (1) {
		rc = getopt(argc, argv, "p:h");
		if (rc < 0)
			break;

		switch (rc) {
			case 'p': // SNPP service or port
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
	fprintf(stderr, "Syntax: %s [-h] [-p SNPP_port]\n", argv[0]);
	exit(EXIT_FAILURE);

}

