// kwfsnpp - Simple Network Paging Protocol gateway to APRS-IS
// Copyright Kenneth Finnegan, 2014
//
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main (int argc, char **argv) {
	int rc;

	while (1) {
		rc = getopt(argc, argv, "h");
		if (rc < 0)
			break;

		switch (rc) {
			case 'h':
				// Do something
				break;
		}
	}

	printf("Hello world!\n");

	return 0;

usage:
	fprintf(stderr, "Syntax: %s [-h]\n", argv[0]);
	exit(1);

}


