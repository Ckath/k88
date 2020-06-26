#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "../../ini_rw/ini_rw.h"

/* required */
#include "../modtape.h"
#include "../../modules.h"
#include "../../irc.h"

static void
handle_cmdmsg(
		irc_conn *s, char *index, char *chan, char *user, char *msg, bool mod)
{
	/* admin only */
	if (!mod) {
		return;
	}

	if (!strncmp(msg, "update", 6)) {
		FILE *cp = popen("git rev-parse --short HEAD", "r");
		char oldver[9] = { '\0' };
		fgets(oldver, 8, cp);

		if(system("git pull -r") || system("make")) {
			send_raw(s, 0, "PRIVMSG %s :something terrible happened trying " \
					"to update, please ssh in to fix it\r\n", DEST, oldver);
			pclose(cp);
			return;
		}

		cp = popen("git rev-parse --short HEAD", "r");
		char newver[9] = { '\0' };
		fgets(newver, 8, cp);
		send_raw(s, 0, "PRIVMSG %s :updated:7 %s ->7 %s\r\n",
				DEST, oldver, newver);
		pclose(cp);
		system("pkill k88");
	} else if (!strncmp(msg, "version", 7)) {
		FILE *cp = popen("git rev-parse --short HEAD", "r");
		char ver[9] = { '\0' };
		fgets(ver, 8, cp);
		send_raw(s, 0, "PRIVMSG %s :current version:7 %s\r\n", DEST, ver);
		pclose(cp);
	}
}

void
gitupdate_init()
{
	mods_new("update", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
