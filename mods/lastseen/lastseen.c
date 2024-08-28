#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"
#include "../../utils/strutils.h"

static INI *seen;

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* TODO: possibly add this on rawmsg, though user lacks there */
	char lastseen[BUFSIZE];
	lowerdup(mi->user, luser);
	sprintf(lastseen, "%u %s <%s> %s", time(NULL), mi->chan, mi->user, msg);
	sini_write(seen, mi->conn->index, luser, lastseen);
	free(luser);
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "seen ", 5)) {
		return;
	}

	/* TODO: add a way to check other servers as well,
	 * or merge all into one entry and dont keep track which server it is */

	/* strip spaces */
	char name[256] = { '\0' };
	strcpy(name, strchr(msg, ' ')+1);
	if (strchr(name, ' ')) {
		strchr(name, ' ')[0] = '\0';
	}

	lowerdup(name, lname);
	char *lastseen = ini_read(seen, mi->conn->index, lname);
	if (lastseen) {
		char timestr[80];
		strncpy(timestr, lastseen, 79);
		lastseen = strchr(lastseen, ' ')+1;
		strchr(timestr, ' ')[0] = '\0';
		strtimef(timestr, time(NULL) - atol(timestr));
		send_privmsg("last msg: %s (%s ago)", lastseen, timestr);
	} else {
		send_privmsg("no data");
	}
	free(lname);
}

void
lastseen_init()
{
	mods_new("lastseen", true);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
	seen = ini_load("mods/lastseen/seen.ini");
}
