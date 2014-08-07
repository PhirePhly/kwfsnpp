// Useful utility functions
// Kenneth Finnegan, 2014

#define BUFFER_LEN	2048

void *get_in_addr(struct sockaddr *sa);

struct recvline_state {
	int fd;
	int start_offset;
	int end_offset;
	char buf[BUFFER_LEN];
};

int recvline(struct recvline_state *s, char **next);

