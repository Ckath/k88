#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "irc.h"
#include "modules.h"
#include "../mods/modtape.h"
#include "../ini_rw/ini_rw.h"

module *all_mods;
module *rawmsg_mods;
module *privmsg_mods;
module *cmdmsg_mods;
module *timed_mods;
int all_mods_len;
int rawmsg_mods_len;
int privmsg_mods_len;
int cmdmsg_mods_len;
int timed_mods_len;
INI *modconf;
char **mods_slist = NULL;

/* these mods_ functions get called from the modules
 * in order to access/alter module related data in this unit */
void
mods_new(char *name, bool default_enable)
{
	all_mods = realloc(all_mods, sizeof(module) * ++all_mods_len);
	strcpy(NEWMOD.name, name);
	NEWMOD.default_enable = default_enable;
}

void
mods_rawmsg_handler(void *handler)
{
	NEWMOD.rawmsg = handler;
}

void
mods_privmsg_handler(void *handler)
{
	NEWMOD.privmsg = handler;
}

void
mods_cmdmsg_handler(void *handler)
{
	NEWMOD.cmdmsg = handler;
}

void
mods_timed_handler(void *handler)
{
	NEWMOD.timed = handler;
}

char *
mods_get_config(char *index, char *item)
{
	return ini_read(modconf, index, item);
}

char *
mods_get_prefix(irc_conn *conn, char *index)
{
	char *prefix = ini_read(modconf, index, "prefix");
	if (!prefix) {
		char *default_prefix = ini_read(conn->globalconf, conn->index, "prefix");
		ini_write(modconf, index, "prefix", default_prefix ? default_prefix : ";");
		prefix = ini_read(modconf, index, "prefix");
	}
	return prefix;
}

int
mods_set_config(char *index, char *item, char *value)
{
	return ini_write(modconf, index, item, value);
}

char **
mods_list()
{
	/* can get away with only generating this once as mods are only
	 * loaded on init, once, at least for now */
	if (mods_slist) {
		return mods_slist;
	}

	int i;
	for (i = 0; i < all_mods_len; ++i) {
		mods_slist = realloc(mods_slist, sizeof(char *)*(i+2));
		mods_slist[i] = malloc(256);
		strcpy(mods_slist[i], all_mods[i].name);
	}
	mods_slist[i] = NULL;
	return mods_slist;
}

void
init_modules(void)
{
	tape_loadmods();
	for (int i = 0; i < all_mods_len; ++i) {
		if (all_mods[i].rawmsg) {
			rawmsg_mods = realloc(rawmsg_mods,
					sizeof(module) * ++rawmsg_mods_len);
			rawmsg_mods[rawmsg_mods_len-1] = all_mods[i];
		} if (all_mods[i].privmsg) {
			privmsg_mods = realloc(privmsg_mods,
					sizeof(module) * ++privmsg_mods_len);
			privmsg_mods[privmsg_mods_len-1] = all_mods[i];
		} if (all_mods[i].cmdmsg) {
			cmdmsg_mods = realloc(cmdmsg_mods,
					sizeof(module) * ++cmdmsg_mods_len);
			cmdmsg_mods[cmdmsg_mods_len-1] = all_mods[i];
		} if (all_mods[i].timed) {
			timed_mods = realloc(timed_mods,
					sizeof(module) * ++timed_mods_len);
			timed_mods[timed_mods_len-1] = all_mods[i];
		}
	}

	modconf = ini_load("mods/modconf.ini");
}

bool
mod_enabled(module *mod, char *index)
{
	char *mod_status = ini_read(modconf, index, mod->name);
	if (!mod_status) { /* first time seeing this index */
		ini_write(modconf, index, mod->name,
				mod->default_enable ? "enabled" : "disabled");
		return mod->default_enable;
	} if (!strcmp(mod_status, "enabled")) {
		return true;
	}
	return false;
}

void
timed_modules(timed_arg *args)
{
	/* this isnt going to be cleaned up, detach it */
	pthread_detach(pthread_self());

	/* time of function call */
	time_t t = time(NULL);

	for (int s = 0; s < args->n; ++s) {
		char index[100];
		strcpy(index, "timed@");
		strcat(index, args->conn[s].index);
		for (int i = 0; i < timed_mods_len; ++i) {
			if (mod_enabled(&timed_mods[i], index)) {
				timed_mods[i].timed(&args->conn[s], index, t);
			}
		}
	}

	/* cleanup thread, probably */
	pthread_exit(NULL);
	free(args);
}

void
handle_modules(mod_arg *args)
{
	/* this isnt going to be cleaned up, detach it */
	pthread_detach(pthread_self());

	/* figure out current index(channel+server) for config logic */
	char index[100] = { '\0' };
	char *chan_start = strchr(args->line, '#');
	if (chan_start && chan_start[1] != ' ') {
		strcpy(index, chan_start);
		strpbrk(index, " \r")[0] = '@';
		strcpy(strchr(index, '@')+1, args->conn->index);
	} else {
		strcpy(index, "misc@");
		strcat(index, args->conn->index);
	}

	/* call all raw handlers */
	for (int i = 0; i < rawmsg_mods_len; ++i) {
		if (mod_enabled(&rawmsg_mods[i], index)) {
			rawmsg_mods[i].rawmsg(args->conn, index, args->line);
		}
	}

	/* filter out info from raw PRIVMSG string */
    char *msgtype = strchr(args->line, ' ') + 1;
	if (msgtype != (char *) 0x1 && !strncmp(msgtype, "PRIVMSG", 7)) {
		char msg[BUFSIZE];
		char user[BUFSIZE];
		char *chan = strchr(msgtype, ' ') + 1;
		strcpy(user, args->line+1);
		strchr(user, '!')[0] = '\0';
		strcpy(msg, strchr(chan, ':')+1);
		strchr(msg, '\r')[0] = '\0';
		strchr(chan, ' ')[0] = '\0';

		/* call all privmsg handlers */
		for (int i = 0; i < privmsg_mods_len; ++i) {
			if (mod_enabled(&privmsg_mods[i], index)) {
				privmsg_mods[i].privmsg(args->conn, index, chan, user, msg);
			}
		}

		/* call all cmdmsg handlers */
		char *modmatch = ini_read(args->conn->globalconf, args->conn->index, "modmatch");
		bool mod = !strncmp(modmatch, args->line, strlen(modmatch));
		char *prefix = mods_get_prefix(args->conn, index);
		if (!strncmp(msg, prefix, strlen(prefix))) {
			char cmdmsg[BUFSIZE];
			strcpy(cmdmsg, msg+strlen(prefix));
			for (int i = 0; i < cmdmsg_mods_len; ++i) {
				if (mod_enabled(&cmdmsg_mods[i], index)) {
					cmdmsg_mods[i].cmdmsg(args->conn, index, chan, user, cmdmsg, mod);
				}
			}
		}
	}

	/* cleanup thread, probably */
	pthread_exit(NULL);
	free(args);
}
