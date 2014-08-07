// kwfsnppd.h
// Kenneth Finnegan, 2014

#define APRSIDENT	"APZKWF"
#define SNPPDVERS	"kwfsnppd 0.0"

#define CALL_LEN	10

// Global daemon state
struct SVRSTATE {
	char *snpp_port; // service or port number
	int snpp_timeout; // Seconds to shutdown idle connections

	char *mess_queue; // Unix socket to pass messages to APRS-IS
	int mess_queue_fd;

	char *aprsis_svr;
	char *aprsis_port;
	char *aprsis_user;
	char *aprsis_pass;
} svrstate;

// Per-client SNPP server state
struct snpp_state {
	int fd; // Client's socket file descriptor
	struct sockaddr_storage addr;
};

void snpp_listen(void);
static void *snpp_client(void *arg);

void aprsis_start(void);
static void *aprsis_client(void *arg);
