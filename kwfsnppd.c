// kwfsnpp - Simple Network Paging Protocol gateway to APRS-IS
// Copyright Kenneth Finnegan, 2014
//
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>


int main (int argc, char **argv) {
	int rc;

	openlog(NULL, LOG_PERROR | LOG_PID, LOG_DAEMON);

	while (1) {
		rc = getopt(argc, argv, "h");
		if (rc < 0)
			break;

		switch (rc) {
			case 'h':
				goto usage;
				break;
		}
	}


	syslog(LOG_DEBUG, "Start of program");

	printf("Hello world!\n");

	return 0;

usage:
	fprintf(stderr, "Syntax: %s [-h]\n", argv[0]);
	exit(1);

}



