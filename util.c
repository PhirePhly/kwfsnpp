// Useful utility functions 
// Kenneth Finnegan, 2014

#include <netinet/in.h>

#include "util.h"

// Extracts IP addr buried in struct sockaddr (thanks Beej)
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
