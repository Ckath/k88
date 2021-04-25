#include <arpa/inet.h>
#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static char *
mumble_ping(const struct sockaddr_in *addr, char *res)
{
	int sock = socket(addr->sin_family, SOCK_DGRAM, 0);
	if(sock < 0) {
		strcpy(res, "error: failed to sock");
		return res;
	}

	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		strcpy(res, "error: failed to sock");
		return res;
	}

	/* generate id */
	unsigned int seed = (unsigned int) time(NULL);
	uint64_t id = rand_r(&seed);
	id <<= 32;
	id |= rand_r(&seed);

	uint32_t buf32;
	uint64_t buf64;

	char request[12];

	/* export type = 0 */
	buf32 = htobe32(0);
	memcpy(request, &buf32, 4);

	/* prepare and make request */
	buf64 = htobe64(id);
	memcpy(request + 4, &buf64, 8);
	if(sendto(sock, request, 12, MSG_CONFIRM, (const struct sockaddr*) addr,
				sizeof(*addr)) != 12) {
		strcpy(res, "error: unable to send request");
		return res;
	}

	/* read response */
	char response[24];
	socklen_t len = 0;
	if(recvfrom(sock, response, 24, MSG_WAITALL, (struct sockaddr*) addr,
				&len) != 24) {
		strcpy(res, "error: unable to receive response");
		return res;
	}


	/* verify ping id */
	memcpy(&buf64, response + 4, 8);
	buf64 = be64toh(buf64);
	if(buf64 != id) {
		close(sock);
		strcpy(res, "error: invalid id in response");
		return res;
	}

	/* collect and format info from response */
	uint8_t version[4];
	memcpy(&version, response, 4);
	memcpy(&buf32, response + 12, 4);
	uint32_t users = be32toh(buf32);
	memcpy(&buf32, response + 16, 4);
	uint32_t slots = be32toh(buf32);
	memcpy(&buf32, response + 20, 4);
	uint32_t bandwidth = be32toh(buf32);

	sprintf(res, "server version: %" PRIu8 ".%" PRIu8 ".%" PRIu8 ", " \
			"users: %" PRIu32 "/%" PRIu32 ", " \
			"bandwidth: %" PRIu32 "b/s",
			version[1], version[2], version[3], users, slots, bandwidth);
	return res;
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "mumble", 6)) {
		return;
	}

	char *arg = strchr(msg, ' ');
	bool link = false;
	char *linked_mumble = mods_get_config(mi->index, "mumble_server");
	uint16_t port = 0;
	char server[BUFSIZE];

	/* find out what to do, poorly */
	if (!arg || strlen(arg) < 3) {
		if (linked_mumble) {
			strcpy(server, linked_mumble);
			arg = NULL; /* quality workaround */
		} else {
			send_privmsg("no mumble linked to %s or specified in command",
					mi->chan);
			return;
		}
	} else if (!strncmp(arg+1, "link", 4)) {
		link = true;
		arg = strchr(arg+1, ' ');
		if (!arg) {
			send_privmsg("no mumble specified to link");
			return;
		}
	} if (arg) {
		arg++;
		strcpy(server, arg);
	}

	/* cleanup server and port if special port is provided */
	if (strchr(server, ' ')) {
		sscanf(strchr(server, ' ')+1, "%" SCNu16, &port);
		strchr(server, ' ')[0] = '\0';
	}

	/* link to channel and bail */
	if (link) {
		char mumble_link[BUFSIZE];
		sprintf(mumble_link, port ? "%s %" PRIu16 : "%s", server, port);
		mods_set_config(mi->index, "mumble_server", mumble_link);
		send_privmsg("mumble '%s' linked to %s", mumble_link, mi->chan);
		return;
	}

	/* (try to) get ip from provided url */
	struct addrinfo *res;
	if(getaddrinfo(server, NULL, NULL, &res)) {
		send_privmsg("error: failed dns lookup");
		return;
	}

	/* prepare addr and ping mumble */
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port ? port : 64738);
	addr.sin_family = ((struct sockaddr_in *) res->ai_addr)->sin_family;
	addr.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;

	char r[BUFSIZE] = {'\0'};
	send_privmsg("%s", mumble_ping(&addr, r));
}

void
mumble_init()
{
	mods_new("mumble", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
