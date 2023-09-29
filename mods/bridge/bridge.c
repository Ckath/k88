#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"


typedef struct bridge {
	char servid_a[33];
	irc_conn *serv_a;
	char chan_a[33];
	char servid_b[33];
	irc_conn *serv_b;
	char chan_b[33];
} bridge;
static bridge *routes;
static bool routes_populated = false;
static INI *routing;

static void
load_routing()
{
	char **bridges = sini_list_sections(routing);
	if (!bridges) {
		if (routes) {
			free(routes);
			routes = NULL;
		}
		return;
	}

	/* realloc routes */
	int size;
	for (size = 0; bridges[size]; ++size);
	routes = realloc(routes, sizeof(bridge)*(size+1));
	routes[size].servid_a[0] = '\0'; /* end indicator */

	/* fill routes */
	for (int b = 0; b < size; ++b) {
		/* set server ids, reset server pointers */
		sscanf(bridges[b], "%[^_]_%s", routes[b].servid_a, routes[b].servid_b);
		routes[b].serv_a = NULL;
		routes[b].serv_b = NULL;

		/* collect channels */
		char **chans = sini_list_items(routing, bridges[b]);
		if (!chans) {
			continue;
		} for (int c = 0; chans[c]; ++c) {
			sprintf(routes[b].chan_a, "#%s", chans[c]);
			sprintf(routes[b].chan_b, "#%s",
					ini_read(routing, bridges[b], chans[c]));
		}
	}

	/* make it clear things are reset */
	routes_populated = false;
}

static void
handle_privmsg(msg_info *mi, char *msg)
{
	if (!routes) { /* nothing to route */
		return;
	}

	/* server conn obtain state
	 * filthy bodge job due to these handlers being per server
	 * server irc_conn pointers are obtained as the privmsgs come by */
	if (!routes_populated) {
		int populated = 0;
		int r;
		for (r = 0; routes[r].servid_a[0]; ++r) {
			/* fill servers if they match current */
			if (!strcmp(routes[r].servid_a, mi->conn->index)) {
				routes[r].serv_a = mi->conn;
			} if (!strcmp(routes[r].servid_b, mi->conn->index)) {
				routes[r].serv_b = mi->conn;
			}

			/* count fully populated routes */
			if (routes[r].serv_a && routes[r].serv_b) {
				populated++;
			}
		}
		/* mark ready when all routes have been populated  */
		if (r == populated) {
			routes_populated = true;
		} else {
			return;
		}
	}


	/* chan_a@serv_a -> chan_b@serv_b forwarding */
	/* technically this could go over max msg length, but I dont care */
	for (int r = 0; routes[r].servid_a[0]; ++r) {
		if (!strcmp(routes[r].servid_a, mi->conn->index) &&
				!strcmp(mi->chan, routes[r].chan_a)) {
			send_raw(routes[r].serv_b, 0, "PRIVMSG %s :<%s> %s\r\n",
					routes[r].chan_b, mi->user, msg);
		}
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	/* mod only zone */
	if (!mi->mod) {
		return;
	}

	/* not optimal cmd logic, awful even, but alas */
	if (!strncmp(msg, "forward ", 8)) {
		char chan[33];
		char serv[33];
		char route_id[33];
		sscanf(strchr(msg, ' ')+1, "%[^@]@%s", chan, serv);
		sprintf(route_id, "%s_%s", mi->conn->index, serv);
		sini_write(routing, route_id, mi->chan+1, chan);
		send_privmsg("route to #%s at %s added", chan, serv);
	} else if (!strncmp(msg, "syphen ", 7)) {
		char chan[33];
		char serv[33];
		char route_id[33];
		sscanf(strchr(msg, ' ')+1, "%[^@]@%s", chan, serv);
		sprintf(route_id, "%s_%s", serv, mi->conn->index);
		sini_write(routing, route_id, chan, mi->chan+1);
		send_privmsg("route from #%s at %s added", chan, serv);
	} else if (!strncmp(msg, "bridge ", 7)) {
		char chan[33];
		char serv[33];
		char route_id[33];
		sscanf(strchr(msg, ' ')+1, "%[^@]@%s", chan, serv);
		sprintf(route_id, "%s_%s", mi->conn->index, serv);
		sini_write(routing, route_id, mi->chan+1, chan);
		sprintf(route_id, "%s_%s", serv, mi->conn->index);
		sini_write(routing, route_id, chan, mi->chan+1);
		send_privmsg("bridge between here and #%s at %s added", chan, serv);
	} else if(!strncmp(msg, "rmforward ", 10) || !strncmp(msg, "delforward ", 11)) {
		char chan[33];
		char serv[33];
		char route_id[33];
		sscanf(strchr(msg, ' ')+1, "%[^@]@%s", chan, serv);
		sprintf(route_id, "%s_%s", mi->conn->index, serv);
		sini_remove(routing, route_id, mi->chan+1);
		send_privmsg("route to #%s at %s removed", chan, serv);
	} else if(!strncmp(msg, "rmsyphen ", 9) || !strncmp(msg, "delsyphen ", 10)) {
		char chan[33];
		char serv[33];
		char route_id[33];
		sscanf(strchr(msg, ' ')+1, "%[^@]@%s", chan, serv);
		sprintf(route_id, "%s_%s", serv, mi->conn->index);
		sini_remove(routing, route_id, chan);
		send_privmsg("route from #%s at %s removed", chan, serv);
	} else if(!strncmp(msg, "rmbridge ", 9) || !strncmp(msg, "delbridge ", 10)) {
		char chan[33];
		char serv[33];
		char route_id[33];
		sscanf(strchr(msg, ' ')+1, "%[^@]@%s", chan, serv);
		sprintf(route_id, "%s_%s", mi->conn->index, serv);
		sini_remove(routing, route_id, mi->chan+1);
		sprintf(route_id, "%s_%s", serv, mi->conn->index);
		sini_remove(routing, route_id, chan);
		send_privmsg("bridge between here and #%s at %s removed", chan, serv);
	} else {
		return;
	}

	load_routing();
}

void
bridge_init()
{
	routing = ini_load("mods/bridge/routing.ini");
	load_routing();
	mods_new("bridge", true);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
}
