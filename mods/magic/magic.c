/* literal copy/paste job from cute/cute.c sed -i 's/cute/magic/g' :shipit:
 * -tso 2023-08-24 7:10:10 AM */
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

/* yes, these are stolen from glitz,
 * they also use the same format to steal more in the future */
static char *target_magics[] = {
    "(つ˵•́ω•̀˵)つ━☆ﾟ.*･｡ﾟ҉̛ {target}",
    "(つ˵•́ω•̀˵)つ━☆✿✿✿✿✿✿ {target}",
    "╰( ´・ω・)つ──☆ﾟ.*･｡ {target}",
    "╰( ´・ω・)つ──☆✿✿✿✿✿✿ {target}",
    "(○´･∀･)o<･。:*ﾟ;+．{target}",
};

static char *magics[] = {
    "pew pew mf :DDD"
}; /* extendable, but not extended */

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "magic", 5)) {
		return;
	}

	srand(time(NULL));
	char magic_pick[BUFSIZE] = {'\0'};
	char *target = strchr(msg, ' ');

	/* prepare the magic, targeted or not */
	strcpy(magic_pick, target ? target_magics[rand()%5] : magics[0]);
	if (target) {
		strrplc(magic_pick, "{target}", target+1);
	}

	send_privmsg("%s", magic_pick);
}

void
magic_init()
{
	mods_new("magic", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
