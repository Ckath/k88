#include <netdb.h>
#include <stdio.h>

#include "socks.h"
#include "log.h"

int
init_sock(int *sock, char *server, char *port)
{
	struct addrinfo hints, *ai;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = 0;

	int ret;
	if ((ret = getaddrinfo(server, port, &hints, &ai))) {
		log_err("failed to get addrinfo: %s\n", gai_strerror(ret));
		return 1;
	}

	if ((*sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) {
		log_err("failed to create socket\n");
		return 1;
	}

	if (connect(*sock, ai->ai_addr, ai->ai_addrlen)) {
		log_err("failed to connect\n");
		return 1;
	}

	freeaddrinfo(ai);

	return 0;
}
