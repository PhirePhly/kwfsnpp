// kwfsnppd.h
// Kenneth Finnegan, 2014

// Global daemon state
struct SVRSTATE {
	char *snpp_port; // service or port number
	int snpp_timeout; // Seconds to shutdown idle connections

	char *mess_queue; // Unix socket to pass messages to APRS-IS
} svrstate;

// Per-client SNPP server state
struct snpp_state {
	int fd; // Client's socket file descriptor
	struct sockaddr_storage addr;

	int mess_queue;

};

void snpp_listen(void);
static void *snpp_client(void *arg);

