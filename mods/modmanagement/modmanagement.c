#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

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
	/* public commands */
	if (!strncmp(msg, "listmods", 8)) {
		char **mods = mods_list();
		char modlist[BUFSIZE] = {'\0'};
		for (int i = 0; mods[i]; ++i) {
			char *mod_status = mods_get_config(index, mods[i]);
			strcat(modlist, mod_status ? strcmp(mod_status, "enabled") ?
					"4" : "3" : "");
			strcat(modlist, mods[i]);
			strcat(modlist, "");
			if (mods[i+1]) {
				strcat(modlist, ", ");
			}
		}
		send_fprivmsg("mods: %s\r\n", modlist);
	}

	/* admin only */
	if (!mod) {
		return;
	} if (!strncmp(msg, "disable ", 8)) {
		char **mods = mods_list();
		char *mod = strchr(msg, ' ')+1;
		char *state = mod_state(index, mods, mod);
		if (!state) {
			send_fprivmsg("%s was not found\r\n", mod);
			return;
		} if (!strcmp(state, "disabled")) {
			send_fprivmsg("%s was already disabled\r\n", mod);
		} else {
			mods_set_config(index, mod, "disabled");
			send_fprivmsg("%s is now4 disabled\r\n", mod);
		}
	} else if (!strncmp(msg, "enable ", 7)) {
		char **mods = mods_list();
		char *mod = strchr(msg, ' ')+1;
		char *state = mod_state(index, mods, mod);
		if (!state) {
			send_fprivmsg("%s was not found in available mods\r\n", mod);
			return;
		} if (!strcmp(state, "enabled")) {
			send_fprivmsg("%s was already enabled\r\n", mod);
		} else {
			mods_set_config(index, mod, "enabled");
			send_fprivmsg("%s is now3 enabled\r\n", mod);
		}
	} else if (!strncmp(msg, "prefix ", 7)) {
		char *oldprefix = strdup(mods_get_prefix(s, index));
		char *newprefix = strchr(msg, ' ')+1;
		mods_set_config(index, "prefix", newprefix);
		send_fprivmsg("prefix: '%s' -> '%s'\r\n", oldprefix, newprefix);
		free(oldprefix);
	}
}

void
modmanagement_init()
{
	mods_new("modmanagement", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
