#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "../../utils/strutils.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (!strncmp(msg, "week", 4)) {
		char buf[2];
		time_t t = time(NULL);
		struct tm *tm_info = localtime(&t);
		strftime(buf, BUFSIZE, "%V", tm_info);
		int w = atoi(buf);

		send_privmsg("%d/52 (%.2f%%)", w, (float)w/52*100);
	}
}

void
date_init()
{
	mods_new("date", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
