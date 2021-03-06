#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "decide ", 7)) {
		return;
	}

	/* replace any " or " with a "," for easy tokenizing */
	char options[BUFSIZE];
	strcpy(options, msg+7);
	char *orpos = strstr(options, " or ");
	if (orpos) {
		strcpy(orpos, ",");
		strcpy(orpos+1, orpos+4);
	}

	srand(time(NULL));
	/* case where only one choice was given */
	if (!strchr(options, ',')) {
		send_privmsg("%s", rand()%2 ? "yes" : "no");
		return;
	}

	/* save an unfucked string of options,
	 * then go through them all until rand()%20 == 8
	 * this is terrible (on purpose) */
	char *restore = strdup(options);
	char *opt = strtok(options, ",");
	int i = 0;
	while (rand()%20 != 8) {
		if (!(opt = strtok(NULL, ","))) {
			strcpy(options, restore);
			opt = strtok(options, ",");
		}
	}
	send_privmsg("%s", opt[0] == ' ' ? opt+1 : opt);
	free(restore);
}

void
decide_init()
{
	mods_new("decide", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
