#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "../../utils/format.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static INI *trigs;

static void
handle_privmsg(msg_info *mi, char *msg)
{
	char **trig_list = ini_list_items(trigs, mi->chan);
	if (!trig_list) {
		return;
	}
	for (int i = 0; trig_list[i]; ++i) {
		char trig_resp[BUFSIZE] = { '\0' };
		strcpy(trig_resp, ini_read(trigs, mi->chan, trig_list[i]));

		/* either match exactly 
		 * or match beginning for ones expecting {arg} */
		bool match = !strcmp(msg, trig_list[i])
				|| (strstr(trig_resp, "{arg}") && 
						!strncmp(msg, trig_list[i], strlen(trig_list[i])));
		if (match) {
			char *arg = strstr(msg, trig_list[i])+strlen(trig_list[i])+1;
			mi_format(trig_resp, mi, arg);
			send_privmsg("%s", trig_resp);
			return;
		}
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "trig ", 5)) {
		return;
	}

	char *cmd = strchr(msg, ' ')+1;
	if (!strncmp(cmd, "add ", 4) || !strncmp(cmd, "set ", 4)) {
		if (!strstr(cmd, " -> ")) {
			send_privmsg("invalid format, it's add ayy -> lmao");
			return;
		}
		char trig_index[BUFSIZE] = { '\0' };
		char trig_resp[BUFSIZE] = { '\0' };
		strcpy(trig_index, strchr(cmd, ' ')+1);
		strcpy(trig_resp, strstr(trig_index, "->")+3);
		strstr(trig_index, " -> ")[0] = '\0';
		ini_write(trigs, mi->chan, trig_index, trig_resp);
		send_privmsg("added");
	} else if(!strncmp(cmd, "rm ", 3) || !strncmp(cmd, "del ", 4)) {
		char *trig_index = strchr(cmd, ' ')+1;
		if (!trig_index) {
			return;
		}
		int r = ini_remove(trigs, mi->chan, trig_index);
		if (r) {
			send_privmsg("removed");
		} else {
			send_privmsg("no such trig");
		}
	} else if(!strncmp(cmd, "list", 4)) {
		char **trig_list = ini_list_items(trigs, mi->chan);
		char triglist[BUFSIZE] = {'\0'};
		for (int i = 0; trig_list[i]; ++i) {
			strcat(triglist, trig_list[i]);
			if (trig_list[i+1]) {
				strcat(triglist, ", ");
			}
		}
		send_privmsg("trigs here: %s", triglist);
	}
}

void
trigs_init()
{
	trigs = ini_load("mods/trigs/trigs.ini");

	mods_new("trigs", true);
	mods_cmdmsg_handler(handle_cmdmsg);
	mods_privmsg_handler(handle_privmsg);
}
