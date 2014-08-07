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
	svrstate.aprsis_svr = "noam.aprs2.net";
	svrstate.aprsis_port = "14580";
	svrstate.aprsis_user = NULL;
	svrstate.aprsis_pass = NULL;

	while (1) {
		rc = getopt(argc, argv, "p:u:s:P:h");
		if (rc < 0)
			break;

		switch (rc) {
			case 'u': // APRS-IS username
				svrstate.aprsis_user = optarg;
				break;
			case 'p': // APRS-IS password
				svrstate.aprsis_pass = optarg;
				break;
			case 's': // APRS-IS upstream server
				svrstate.aprsis_svr = optarg;
			case 'P': // SNPP service or port
				svrstate.snpp_port = optarg;
				break;
			case 'h':
			default:
				goto usage;
		}
	}

	if (svrstate.aprsis_user == NULL) {
		fprintf(stderr, "ERROR: Missing APRS-IS login callsign\n");
		goto usage;
	}
	if (svrstate.aprsis_pass == NULL) {
		fprintf(stderr, "ERROR: Missing APRS-IS login password\n");
		goto usage;
	}

	// spawn APRS-IS helper thread
	aprsis_start();

	// Block on SNPP service and create new SNPP client threads ad-hoc
	snpp_listen();

	return 0;

usage:
	fprintf(stderr, "Syntax: %s -u MYCALL -p 12345\n"
			"\t\t[-h] [-P SNPP_port]\n", argv[0]);
	exit(EXIT_FAILURE);

}

