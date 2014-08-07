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
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "kwfsnppd.h"
#include "util.h"

void aprsis_start () {

	pthread_t tclient;

	pthread_create(&tclient, NULL, aprsis_client, NULL);
	pthread_detach(tclient);

	return;
}

static void * aprsis_client(void *arg) {
	int rc;

	syslog(LOG_DEBUG, "Connecting to APRS-IS server: %s:%s", 
			svrstate.aprsis_svr, svrstate.aprsis_port);

	return NULL;
}

// Connect and authenticate to an APRS-IS server
// Failing to authenticate is a fatal error
int aprsis_connect(void) {
	int rc;


	// Login to connected server
	// send "user %s pass %s vers SNPPDVERS"
	
	return -1;
}

