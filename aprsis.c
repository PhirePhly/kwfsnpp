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

	aprsis_connect();

	// Infinite loop
	while (1) {
		sleep(1);

		// Process incoming packets
		aprsis_rcvr();

		// Send messages in job queue
		aprsis_xmit();
	}
	return NULL;
}

// Connect and authenticate to an APRS-IS server
// Failing to authenticate is a fatal error
int aprsis_connect(void) {
	int rc, newfd;
	syslog(LOG_DEBUG, "(Not actually) Connecting to APRS-IS server: %s:%s", 
			svrstate.aprsis_svr, svrstate.aprsis_port);

	if (svrstate.aprsis_fd != 0) {
		close (svrstate.aprsis_fd);
	}

	struct addrinfo hints, *aprssvr, *p;

	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Get linked list of APRS-IS server addresses
	rc = getaddrinfo(svrstate.aprsis_svr, svrstate.aprsis_port, 
			&hints, &aprssvr);
	for (p = aprssvr; p != NULL; p = p->ai_next) {
		newfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (newfd == -1) {
			continue;
		}

		rc = connect(newfd, p->ai_addr, p->ai_addrlen);
		if (rc == -1) {
			close(newfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		syslog(LOG_ERR, "Failed to connect to APRS-IS server!");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(aprssvr);

	// Login to connected server
	// send "user %s pass %s vers SNPPDVERS"
	char buf[1024];
	sprintf(buf, "user %s pass %s vers %s\r\n", svrstate.aprsis_user,
			svrstate.aprsis_pass, SNPPDVERS);

	printf("Sending login: %s", buf);
	nsend(newfd, buf, strlen(buf));
	svrstate.aprsis_fd = newfd;
	
	memset(&(svrstate.aprsis_buf), 0, sizeof(struct recvline_state));
	svrstate.aprsis_buf.fd = svrstate.aprsis_fd;
	return 0;
}

// Process incoming packets from the APRS-IS server
int aprsis_rcvr(void) {
	int rc;
	char *line, *infofld;

	while ((rc = recvline(&(svrstate.aprsis_buf), &line))) {
		if (rc == -1) {
			printf("Server problems\n");
		}

		printf("Rcvd: %s\n", line);

		if (line[0] == '#') { // Server comment
			continue;
		}

		infofld = strchr(line, ':');
		if (infofld == NULL) { // No info field?
			continue;
		}
		if (strlen(infofld) < 2) { // No APRS field ident?
			continue;
		}
		if (infofld[1] != ':') { // Not a message packet
			continue;
		}

		char smallbuf[1024];
		pad_callsign(smallbuf, svrstate.aprsis_user);
		if (strncasecmp(infofld + 1, smallbuf, 11) != 0) { // Message not to us!
			continue;
		}
		// "::         :ack0000
		if (strncasecmp(infofld + 12, "ack", 3) == 0 ||
				strncasecmp(line + 12, "rej", 3) == 0) { // They're acking...
			unsigned int messid;
			rc = sscanf(infofld + 15, "%x", &messid);
			printf("Ack for mess #%04X\n", messid);
			
			struct message_t *p = svrstate.messqueue.head;
			while (p != NULL) {
				if (p->id == messid) {
					p->state = ACKED;
					break;
				}
				p = p->next;
			}
		} else { // Shit! They've sent us a message!
			// TODO: rej the message...
		}

	}

	return 0;
}

// Transmit any queued messages that are past due
int aprsis_xmit(void) {
	struct message_t *newmess;
	char buf[1024], smallbuf[1024];

	while ((newmess = aprsis_popqueue()) != NULL) {

		if (newmess->state == ACKED) {
			free(newmess);
			continue;
		}

		pad_callsign(smallbuf, newmess->call);
		sprintf(buf, "%s>APZKWF:%s%s {%04X\r\n", svrstate.aprsis_user,
				smallbuf, newmess->mess, newmess->id);
		syslog(LOG_DEBUG, "Send [%d] %s\n", 
				newmess->send_count + 1, buf);

		nsend(svrstate.aprsis_fd, buf, strlen(buf));

		newmess->send_count++;
		newmess->next_try = time(NULL) + about(30*newmess->send_count);
		if (newmess->send_count < svrstate.mess_max_retry) {
			aprsis_enqueue(newmess);
		} else {
			free(newmess);
		}
	}

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

