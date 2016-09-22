// kwfsnppd.h
// Kenneth Finnegan, 2014

#ifndef _KWFSNPPD_H_
#define _KWFSNPPD_H_

#define APRSIDENT	"APZKWF"
#define SNPPDVERS	"kwfsnppd 0.0"

#include "util.h"

// Global daemon state
struct SVRSTATE {
	char *snpp_port; // service or port number
	int snpp_timeout; // Seconds to shutdown idle connections

	int mess_max_retry;

	char *aprsis_svr; // Server hostname
	char *aprsis_port; // Server port service desc
	char *aprsis_user; // APRS-IS login username
	char *aprsis_pass; // APRS-IS login password
	int aprsis_fd;
	time_t aprsis_beacon; // When to next beacon
	char *aprsis_beacontxt;
	struct recvline_state aprsis_buf;

	struct {
		pthread_mutex_t mutex;
		struct message_t *head;
		uint16_t nextid;
	} messqueue;
} svrstate;

// Per-client SNPP server state
struct snpp_state {
	int fd; // Client's socket file descriptor
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN+1];
};

// State machine for each queued message
enum mess_state {
	START,
	RETRY,
	ACKED
};

// Queued message context
struct message_t {
	struct message_t *next;

	enum mess_state state;
	time_t next_try;
	int send_count;
	char call[CALLSIGN_MAXLEN+1]; // Not space padded
	char mess[MESS_MAXLEN+1];
	uint16_t id;
};

void snpp_listen(void);
static void *snpp_client(void *arg);

// APRS-IS public & private functions
void aprsis_start(void);
static void *aprsis_client(void *arg);
int aprsis_connect(void);
int aprsis_rcvr(void);
int aprsis_xmit(void);
int aprsis_createmess(char *call, char *mess);
int aprsis_rejmess(struct tnc2_message *mess);
int aprsis_beacon(void);
int aprsis_enqueue(struct message_t *newmess);
struct message_t *aprsis_popqueue(void);

#endif

