#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
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
handle_cmdmsg(msg_info *mi, char *msg)
{
	/* public commands */
	if (!strncmp(msg, "listmods", 8)) {
		char **mods = mods_list();
		char modlist[BUFSIZE] = {'\0'};
		for (int i = 0; mods[i]; ++i) {
			char *mod_status = mods_get_config(mi->index, mods[i]);
			strcat(modlist, mod_status ? strcmp(mod_status, "enabled") ?
					"4" : "3" : "");
			strcat(modlist, mods[i]);
			strcat(modlist, "");
			if (mods[i+1]) {
				strcat(modlist, ", ");
			}
		}
		send_privmsg("mods: %s", modlist);
	}

	/* admin only */
	if (!mi->mod) {
		return;
	} if (!strncmp(msg, "disable ", 8)) {
		char **mods = mods_list();
		char *mod = strchr(msg, ' ')+1;
		char *state = mod_state(mi->index, mods, mod);
		if (!state) {
			send_privmsg("%s was not found", mod);
			return;
		} if (!strcmp(state, "disabled")) {
			send_privmsg("%s was already disabled", mod);
		} else {
			mods_set_config(mi->index, mod, "disabled");
			send_privmsg("%s is now4 disabled", mod);
		}
	} else if (!strncmp(msg, "enable ", 7)) {
		char **mods = mods_list();
		char *mod = strchr(msg, ' ')+1;
		char *state = mod_state(mi->index, mods, mod);
		if (!state) {
			send_privmsg("%s was not found in available mods", mod);
			return;
		} if (!strcmp(state, "enabled")) {
			send_privmsg("%s was already enabled", mod);
		} else {
			mods_set_config(mi->index, mod, "enabled");
			send_privmsg("%s is now3 enabled", mod);
		}
	} else if (!strncmp(msg, "prefix ", 7)) {
		char *oldprefix = strdup(mods_get_prefix(mi->conn, mi->index));
		char *newprefix = strchr(msg, ' ')+1;
		mods_set_config(mi->index, "prefix", newprefix);
		send_privmsg("prefix: '%s' -> '%s'", oldprefix, newprefix);
		free(oldprefix);
	}
}

void
modmanagement_init()
{
	mods_new("modmanagement", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
