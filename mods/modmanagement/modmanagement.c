#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../modules.h"
#include "../../irc.h"

static char *
mod_state(char *index, char **mods, char *mod)
{
	for (int i = 0; mods[i]; ++i) {
		if (!strcmp(mods[i], mod)) {
			return mods_get_config(index, mod);
		}
	}
	return NULL;
}

static void
handle_cmdmsg(
		irc_conn *s, char *index, char *chan, char *user, char *msg, bool mod)
{
	/* admin only */
	if (!mod) {
		return;
	}

	if (!strncmp(msg, "listmods", 8)) {
		char **mods = mods_list();
		char modlist[2000] = {'\0'};
		for (int i = 0; mods[i]; ++i) {
			strcat(modlist, strcmp(mods_get_config(index, mods[i]), "enabled") ?
					"4" : "3");
			strcat(modlist, mods[i]);
			strcat(modlist, "");
			if (mods[i+1]) {
				strcat(modlist, ", ");
			}
		}
		send_raw(s, 0, "PRIVMSG %s :mods: %s\r\n", DEST, modlist);
	} else if (!strncmp(msg, "disable ", 8)) {
		char **mods = mods_list();
		char *mod = strchr(msg, ' ')+1;
		char *state = mod_state(index, mods, mod);
		if (!state) {
			send_raw(s, 0, "PRIVMSG %s :mod %s not found\r\n", DEST, mod);
			return;
		} if (!strcmp(state, "disabled")) {
			send_raw(s, 0, "PRIVMSG %s :mod %s already disabled\r\n", DEST, mod);
		} else {
			mods_set_config(index, mod, "disabled");
			send_raw(s, 0, "PRIVMSG %s :mod %s disabled\r\n", DEST, mod);
		}
	} else if (!strncmp(msg, "enable ", 7)) {
		char **mods = mods_list();
		char *mod = strchr(msg, ' ')+1;
		char *state = mod_state(index, mods, mod);
		if (!state) {
			send_raw(s, 0, "PRIVMSG %s :mod %s not found\r\n", DEST, mod);
			return;
		} if (!strcmp(state, "enabled")) {
			send_raw(s, 0, "PRIVMSG %s :mod %s already enabled\r\n", DEST, mod);
		} else {
			mods_set_config(index, mod, "enabled");
			send_raw(s, 0, "PRIVMSG %s :mod %s enabled\r\n", DEST, mod);
		}
	}
}

void
modmanagement_init()
{
	mods_new("modmanagement", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
