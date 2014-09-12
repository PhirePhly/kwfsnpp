// Useful utility functions 
// Kenneth Finnegan, 2014

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "util.h"

// Converts sockaddr into a string. 
// 	buf should be at least INET6_ADDRSTRLEN long.
int get_addr_str(char *buf, size_t len, struct sockaddr *sa) {
	struct sockaddr *addr;

	if (sa->sa_family == AF_INET) {
		addr = (struct sockaddr*) &(((struct sockaddr_in*)sa)->sin_addr);
	} else {
		addr = (struct sockaddr*) &(((struct sockaddr_in6*)sa)->sin6_addr);
	}

	inet_ntop(sa->sa_family, addr, buf, len);
	return 0;
}

// recvline - returns a single line at a time for processing
// from a socket file desc.
//
// Returns 1 and sets **next to point to \0-terminated line
// Returns 0 when there are no available lines
// Returns -1 when there are socket problems or user input is too long
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
		if (s->buf[s->start_offset] == '\n' || 
				s->buf[s->start_offset] == '\r') {
			s->start_offset++;
		} else {
			break;
		}
	}

	return 1;
}

// Run through a string to find the second word after whitespace
// "example string" "badexample" "another example string"
//          ^                  ^          ^
char *find_argument(char *line) {

	while (*line != '\0' && !isspace(*line))
		line++;

	while (*line != '\0' && isspace(*line))
		line++;

	return line;
}

// Wrapper around send() to handle any recoverable errors
int nsend(int fd, const char *buf, size_t len) {
	int rc;

	while (len > 0) {
		rc = send(fd, buf, len, 0);

		if (rc == -1) {
			if (errno == EAGAIN || errno == EINTR) {
				continue;
			} else {
				return -1;
			}
		}

		buf += rc;
		len -= rc;
	}

	return 0;
}

// Check if is valid callsign per APRS-IS
int qualify_callsign (char *call) {
	int i;
	int len = strlen(call);
	if (len < 3)
		return -1;
	if (len > 9)
		return -1;

	// Rule out invalid callsigns and ones we aren't willing to handle
	if (strncasecmp(call, "N0CALL", 6) == 0)
		return -1;
	if (strncasecmp(call, "NOCALL", 6) == 0)
		return -1;
	if (strncasecmp(call, "MYCALL", 6) == 0)
		return -1;
	if (strncasecmp(call, "BLN", 3) == 0)
		return 1;

	for (i=0; i< len; i++) {
		call[i] = toupper(call[i]);
	}
	return 0;
}

// Check a message to see if it is a valid APRS message
int qualify_message(char *mess) {
	// Is the message too long?
	if (strlen(mess) > 67) {
		return -1;
	}
	// Are there any unprintable ASCII charaters?
	// TODO
	// Are there any disallowed characters? '{','|','~'
	// TODO
	return 0;
}

// space-pad a 9 character callsign and add :xx: wrapper
int pad_callsign (char *target, char *input) {
	if (strlen(input) > 9)
		return -1;

	sprintf(target, ":         :");
	memcpy(target+1, input, strlen(input));
	return 0;
}

// Given a value, dither it between 100% and 112% its value
int about(int value) {
	value += random() % (value / 8);
	return value;
}

