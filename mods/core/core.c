#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

static void
handle_timed(irc_conn *s, char *index, time_t time)
{
	if (time - s->heartbeat > 300) {
		s->heartbeat = time;
		fputs("[ !!! ] connection timed out, resetting\n", stderr);
		reconnect_conn(s);
	}

	/* touch file for watchdog lockup check */
	if (!(time%60)) {
		fclose(fopen("/tmp/k88_alive", "w+"));
	}
}

static void
handle_rawmsg(msg_info *mi, char *line)
{
	mi->conn->heartbeat = time(NULL);
	if (!strncmp(line, "PING ", 5)) {
		line[1] = 'O';
		send_raw(mi->conn, 0, line);
	} else if(!strncmp(line, "ERROR", 5)) {
		fputs("[ !!! ] recieved ERROR, resetting connection", stderr);
		reconnect_conn(mi->conn);
	} else if (!mi->conn->init && strstr(line, " MODE ")) {
		join_chans(mi->conn, ini_read(mi->conn->globalconf,
					mi->conn->index, "chans"));
		mi->conn->init = 1;
		mi->conn->heartbeat = time(NULL);
	}
}

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* IBIP */
	if (!strncmp(msg, ".bots", 5)) {
		send_privmsg("Reporting in! [C] fix shit: https://github.com/ckath/k88\r\n");
	}

	/* ctcp */
	if (!strncmp(msg, "VERSION", 9)) {
		puts("[ (!) ] ctcp version"); 
		send_notice("VERSION socket.h\r\n");
	} else if (!strncmp(msg, "PING ", 6)) {
		puts("[ (!) ] ctcp ping"); 
		send_fnotice("PING %u\r\n", 88);
	} else if (!strncmp(msg, "TIME", 6)) {
		puts("[ (!) ] ctcp time"); 
		send_fnotice("TIME %u\r\n", (unsigned)time(NULL));
	}
}

void
core_init()
{
	mods_new("core", true);
	mods_timed_handler(handle_timed);
	mods_rawmsg_handler(handle_rawmsg);
	mods_privmsg_handler(handle_privmsg);
}
