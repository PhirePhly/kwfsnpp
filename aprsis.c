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
		sleep(1);
		struct message_t *newmess;
		
		while ((newmess = aprsis_popqueue()) != NULL) {
			printf("%s: %s {%04X\n", newmess->call, newmess->mess,
					newmess->id);

			newmess->send_count++;
			newmess->next_try = time(NULL) + about(30*newmess->send_count);
			if (newmess->send_count < 10) {
				aprsis_enqueue(newmess);
			} else {
				free(newmess);
			}
		}
	}
	return NULL;
}

// Connect and authenticate to an APRS-IS server
// Failing to authenticate is a fatal error
int aprsis_connect(void) {
	int rc;


	// Login to connected server
	// send "user %s pass %s vers SNPPDVERS"
	
	return 0;
}

int aprsis_createmess(char *call, char *mess) {

	// Populate new message_t
	struct message_t *newmess = (struct message_t*)
		malloc(sizeof(struct message_t));
	memset(newmess, 0, sizeof(struct message_t));

	newmess->next = NULL;
	newmess->state = START;
	newmess->next_try = time(NULL);
	newmess->send_count = 0;
	sprintf(newmess->call, "%s", call);
	sprintf(newmess->mess, "%s", mess);
	
	// Fetch new message ID number to identify message
	pthread_mutex_lock(&(svrstate.messqueue.mutex));
	newmess->id = svrstate.messqueue.nextid++;
	pthread_mutex_unlock(&(svrstate.messqueue.mutex));
	
	// enqueue the new message
	aprsis_enqueue(newmess);
	
	return 0;
}

// Add new message to the ordered queue based on epoch next task
int aprsis_enqueue(struct message_t *newmess) {
	newmess->next = NULL;

	pthread_mutex_lock(&(svrstate.messqueue.mutex));

	// Add newmess to empty linked list
	if (svrstate.messqueue.head == NULL) {
		svrstate.messqueue.head = newmess;
		goto done;
	}
	// Add to head of list
	if (newmess->next_try < svrstate.messqueue.head->next_try) {
		newmess->next = svrstate.messqueue.head;
		svrstate.messqueue.head = newmess;
		goto done;
	}

	// Itterate over the whole list to find its rightful place
	struct message_t *l = svrstate.messqueue.head;
	while (l->next && newmess->next_try > l->next->next_try) {
		l = l->next;
	}

	newmess->next = l->next;
	l->next = newmess;
done:
	pthread_mutex_unlock(&(svrstate.messqueue.mutex));
	return 0;
}

// Check the queue for any messages past due to be processed
// If so, remove them from the head and return them
// otherwise, return NULL
struct message_t *aprsis_popqueue(void) {
	struct message_t *nextmess = NULL;
	time_t now;
	// Lock svrstate.messqueue.mutex
	pthread_mutex_lock(&(svrstate.messqueue.mutex));
	
	// Get current time
	now = time(NULL);
	// Check first message->next_try to see if it is time to try again
	if ((svrstate.messqueue.head != NULL) && 
			(svrstate.messqueue.head->next_try <= now)) {
		nextmess = svrstate.messqueue.head;
		svrstate.messqueue.head = nextmess->next;
		nextmess->next = NULL;
	}

	pthread_mutex_unlock(&(svrstate.messqueue.mutex));
	return nextmess;
}

