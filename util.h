// Useful utility functions
// Kenneth Finnegan, 2014

#define BUFFER_LEN	2048

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

// Wrapper for send() for recoverable errors
int nsend(int fd, const char *buf, size_t len);

