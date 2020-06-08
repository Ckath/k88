#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../modules.h"
#include "../../irc.h"

#include "../../ini_rw/ini_rw.h"

static void
handle_rawmsg(irc_conn *s, char *index, char *line)
{
	if (!strncmp(line, "PING ", 5)) {
		line[1] = 'O';
		send_raw(s, 0, line);
	} else if(!strncmp(line, "ERROR", 5)) {
		fputs("[ !!! ] recieved ERROR, resetting connection", stderr);
		destroy_conn(s);
		init_conn(s);
	} else if (!s->init && strstr(line, " MODE ")) {
		INI *conf = ini_load("config.ini");
		join_chans(s, ini_read(conf, s->index, "chans"));
		ini_free(conf);
		s->init = 1;
	}
}

static void
handle_privmsg(irc_conn *s, char *index, char *chan, char *user, char *msg)
{
	/* IBIP */
	if (!strncmp(msg, ".bots", 5)) {
		send_raw(s, 0, "PRIVMSG %s :Reporting in! [C] https://github.com/ckath/k88\r\n", DEST); 
	}

	/* ctcp */
	if (!strncmp(msg, "VERSION", 9)) {
		puts("[ (!) ] ctcp version"); 
		send_raw(s, 0, "NOTICE %s :VERSION socket.h\r\n", DEST);
	} else if (!strncmp(msg, "PING ", 6)) {
		puts("[ (!) ] ctcp ping"); 
		send_raw(s, 0, "NOTICE %s :PING %u\r\n", DEST, 88);
	} else if (!strncmp(msg, "TIME", 6)) {
		puts("[ (!) ] ctcp time"); 
		send_raw(s, 0, "NOTICE %s :TIME %u\r\n", DEST, (unsigned)time(NULL));
	}
}

void
core_init()
{
	mods_new("core", true);
	mods_rawmsg_handler(handle_rawmsg);
	mods_privmsg_handler(handle_privmsg);
}
