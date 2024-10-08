#define _GNU_SOURCE /* strcasecmp */
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

static INI *tell;
static char **nicks;

static void
handle_privmsg(msg_info *mi, char *msg)
{
	if (!nicks) {
		return;
	}

	/* check if nick has messages */
	char user[256];
	char timestr[80];
	strcpy(user, mi->user);
	bool nick_popped = false;
	for (int n = 0; nicks[n]; ++n) {
		if (!strcasecmp(nicks[n], mi->user)) {
			char **msgs = sini_list_items(tell, nicks[n]);
			for (int m = 0; msgs[m]; ++m) {
				send_privmsg("%s: %s (%s ago)", mi->user,
						ini_read(tell, nicks[n], msgs[m]),
						strtimef(timestr, time(NULL)-atol(msgs[m])));
				sini_remove(tell, nicks[n], msgs[m]);
			}
			nick_popped = true;
		}
	}

	if (nick_popped) {
		nicks = sini_list_sections(tell);
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "tell ", 5)) {
		return;
	}

	/* strip spaces */
	char name[256] = { '\0' };
	strcpy(name, strchr(msg, ' ')+1);
	if (strchr(name, ' ')) {
		strchr(name, ' ')[0] = '\0';
	}

	char timestr[80];
	sprintf(timestr, "%u", time(NULL));
	char tellmsg[BUFSIZE];
	sprintf(tellmsg, "<%s> %s", mi->user, strchr(strchr(msg, ' ')+1, ' ')+1);
	sini_write(tell, name, timestr, tellmsg);

	nicks = sini_list_sections(tell);
	send_privmsg("might do");
}

void
tell_init()
{
	mods_new("tell", true);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
	tell = ini_load("mods/tell/tells.ini");
	nicks = sini_list_sections(tell);
}
