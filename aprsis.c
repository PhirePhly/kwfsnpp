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
	int rc;
	pthread_t tclient;

	rc = pthread_mutex_init(&(svrstate.messqueue.mutex), NULL);
	svrstate.messqueue.head = NULL;

	pthread_create(&tclient, NULL, aprsis_client, NULL);
	pthread_detach(tclient);

	return;
}

static void * aprsis_client(void *arg) {
	int rc;

	syslog(LOG_DEBUG, "(Not actually) Connecting to APRS-IS server: %s:%s", 
			svrstate.aprsis_svr, svrstate.aprsis_port);
	aprsis_connect();

	while (1) {
		sleep(5);
		aprsis_checkqueue();
	}
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

int aprsis_createmess(char *call, char *mess) {
	// Fill out new message_t
	// lock svrstate.messqueue.mutex
	// Aquire and increment nextid
	// unlock svrstate.messqueue.mutex
	// set the next attempt at "now"
	// enqueue the new message
	return 0;
}

// Add new message to the ordered queue based on epoch next task
int aprsis_enqueue(struct message_t *newmess) {

	// Lock svrstate.messqueue.mutex
	// Add newmess to linked list
	// Unlock svrstate.messqueue.mutex
	return 0;
}

// Check the queue for any messages past due to be retried
int aprsis_checkqueue(void) {
	// Lock svrstate.messqueue.mutex
	// Get current time
	// Check first message->next_try to see if it is time to try again
	// send it
	// Increment message->next_try & send_count
	// Readd it to the queue
	// Unlock svrstate.messqueue.mutex
	return 0;
}

