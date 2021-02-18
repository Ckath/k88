#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static INI *seen;

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* TODO: possibly add this on rawmsg, though user lacks there */
	char lastseen[BUFSIZE];
	char timestr[80];
	time_t t = time(NULL);
	strftime(timestr, 80, "%X %x %Z", localtime(&t));
	sprintf(lastseen, "%s <%s> %s (%s)", mi->chan, mi->user, msg, timestr);
	ini_write(seen, mi->conn->index, mi->user, lastseen);
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

	char *lastseen = ini_read(seen, mi->conn->index, name);
	if (lastseen) {
		send_privmsg("last msg: %s", lastseen);
	} else {
		send_privmsg("no data", lastseen);
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
