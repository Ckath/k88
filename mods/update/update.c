#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "../../ini_rw/ini_rw.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	/* admin only */
	if (!mi->mod) {
		return;
	}

	if (!strncmp(msg, "update", 6)) {
		FILE *cp = popen("git rev-parse --short HEAD", "r");
		char oldver[9] = { '\0' };
		fgets(oldver, 8, cp);

		if(system("git pull -r") ||
				system("git submodule update --recursive") ||
				system("make")) {
			send_privmsg("something terrible happened trying to update, " \
					"please ssh in to fix it\r\n");
			pclose(cp);
			return;
		}

		cp = popen("git rev-parse --short HEAD", "r");
		char newver[9] = { '\0' };
		fgets(newver, 8, cp);
		if (!strcmp(oldver, newver)) {
			send_privmsg("git up to date, recompiled anyway\r\n");
		} else {
			send_fprivmsg("updated:7 %s ->7 %s\r\n", oldver, newver);
		}
		pclose(cp);
		system("pkill k88");
	} else if (!strncmp(msg, "version", 7)) {
		FILE *cp = popen("git rev-parse --short HEAD", "r");
		char ver[9] = { '\0' };
		fgets(ver, 8, cp);
		send_fprivmsg("current version:7 %s\r\n", ver);
		pclose(cp);
	}
}

void
gitupdate_init()
{
	mods_new("update", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
