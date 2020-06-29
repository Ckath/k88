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
handle_cmdmsg(
		irc_conn *s, char *index, char *chan, char *user, char *msg, bool mod)
{
	/* admin only */
	if (!mod) {
		return;
	}

	if (!strncmp(msg, "shell ", 6)) {
		char *cmd = strchr(msg, ' ')+1;
		FILE *cp = popen(cmd, "r");
		char output[500] = { '\0' };
		fgets(output, 499, cp);
		if (output[0]) {
			send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, output);
		}
		pclose(cp);
	}
}

void
adminutils_init()
{
	mods_new("adminutils", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
