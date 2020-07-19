#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

INI *seen;

static void
handle_privmsg(irc_conn *s, char *index, char *chan, char *user, char *msg)
{
	/* TODO: possibly add this on rawmsg, though user lacks there */
	char lastseen[BUFSIZE];
	char timestr[80];
	time_t t = time(NULL);
	strftime(timestr, 80, "%X %x %Z", localtime(&t));
	sprintf(lastseen, "%s <%s> %s (%s)", chan, user, msg, timestr);
	ini_write(seen, s->index, user, lastseen); 
}

static void
handle_cmdmsg(
		irc_conn *s, char *index, char *chan, char *user, char *msg, bool mod)
{
	if (strncmp(msg, "seen ", 5)) {
		return;
	}

	/* TODO: add a way to check other servers as well, 
	 * or merge all into one entry and dont keep track which server it is */
	char *lastseen = ini_read(seen, s->index, strchr(msg, ' ')+1);
	if (lastseen) {
		send_fprivmsg("last msg: %s\r\n", lastseen);
	} else {
		send_fprivmsg("no data\r\n", lastseen);
	}
}

void
lastseen_init()
{
	mods_new("lastseen", true);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
	seen = ini_load("mods/lastseen/seen.ini");
}
