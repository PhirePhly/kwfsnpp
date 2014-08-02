// kwfsnpp - Simple Network Paging Protocol gateway to APRS-IS
// Copyright Kenneth Finnegan, 2014
//
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "kwfsnppd.h"

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
	syslog(LOG_DEBUG, "Binding to SNPP port %s", svrstate.snpp_port);
}
