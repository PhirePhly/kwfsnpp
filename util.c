// Useful utility functions 
// Kenneth Finnegan, 2014

#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "util.h"

// Extracts IP addr buried in struct sockaddr (thanks Beej)
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int recvline(struct recvline_state *s, char **next) {
	int rc, i;

	// Shift everything forward if we're in the back half
	if (s->start_offset > (BUFFER_LEN / 2)) {
		memmove(s->buf, s->buf + s->start_offset, 
				s->end_offset - s->start_offset);
		s->end_offset -= s->start_offset;
		s->start_offset = 0;
	}

	// If we have any room, read in more data
	if (s->end_offset < BUFFER_LEN) {
		rc = recv(s->fd, s->buf + s->end_offset,
				BUFFER_LEN - s->end_offset, MSG_DONTWAIT);
		if (rc == -1) {
			if (errno != EAGAIN && errno != EINTR) {
				return -1;
			}
		} else {
			s->end_offset += rc;
		}
	}

	// Find the next newline
	for (i = s->start_offset; i < s->end_offset; i++) {
		if (s->buf[i] == '\n' || s->buf[i] == '\r') {
			break;
		}
	}

	if (i == s->end_offset) {
		if (s->end_offset - s->start_offset > (BUFFER_LEN / 2)) {
			return -1; // Unreasonably long line
		} else {
			return 0;
		}
	}

	s->buf[i] = '\0';
	*next = s->buf + s->start_offset;
	s->start_offset = i + 1;

	while (s->start_offset < s->end_offset) {
		if (s->buf[s->start_offset] == '\n' || s->buf[s->start_offset] == '\r') {
			s->start_offset++;
		} else {
			break;
		}
	}

	return 1;
}

