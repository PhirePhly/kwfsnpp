// Useful utility functions
// Kenneth Finnegan, 2014

#define BUFFER_LEN	2048
#define CALLSIGN_LEN	(9+2+1)

// Convert struct sockaddr into string
int get_addr_str(char *buf, size_t len, struct sockaddr *sa);

struct recvline_state {
	int fd;
	int start_offset;
	int end_offset;
	char buf[BUFFER_LEN];
};

// Return lines from recvline_state.fd one line at a time
int recvline(struct recvline_state *s, char **next);

// Run through a string to find the second word after whitespace
// "example string" "badexample"
//          ^                  ^
char *find_argument(char *line);

// Wrapper for send() for recoverable errors
int nsend(int fd, const char *buf, size_t len);

// Qualify a string as a valid APRS-IS username
// Returns -1 if that isn't the case
int qualify_callsign(char *call);

int qualify_message(char *mess);

// Pads a callsign out to the 9 characters and ::'s required by APRS
int pad_callsign(char *target, char *input);

