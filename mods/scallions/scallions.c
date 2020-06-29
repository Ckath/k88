#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

#include "../../ini_rw/ini_rw.h"

INI *lookup;
char **urls;

static void
handle_privmsg(irc_conn *s, char *index, char *chan, char *user, char *msg)
{
	/* handle all the onions */
	for (int i = 0; urls[i]; ++i) {
		char *match = strstr(msg, urls[i]);
		if (match) {
			char tmp[BUFSIZE] = {'\0'};
			strncpy(tmp, msg, match-msg);
			strcat(tmp, ini_read(lookup, "onions", urls[i]));
			strcat(tmp, match+strlen(urls[i]));
			send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
		}
	}

	/* hardcoded twitter img redirect, pretty useless */
	char *match = strstr(msg, "https://pbs.twimg.com/media/");
	if (match) {
		char tmp[BUFSIZE] = {'\0'};
		strncpy(tmp, msg, match-msg);
		strcat(tmp, "https://nitter.net/pic/media/");
		strcat(tmp, match+28);

		char *tail = strchr(tmp, '?');
		if (tail) {
			strncpy(tail, ".png", 4); 
			char *leftovermsg = strchr(match, ' ');
			strcpy(tail+4, leftovermsg ? leftovermsg : "\r\n\0");
		}
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
	}
}


static void
handle_cmdmsg(
		irc_conn *s, char *index, char *chan, char *user, char *msg, bool mod)
{
	/* admin only */
	if (!mod) {
		return;
	}

	if (!strncmp(msg, "onion ", 6)) {
		char onion[BUFSIZE];
		strcpy(onion, strchr(msg, ' ')+1);
		strchr(onion, ' ')[0] = '\0'; 

	    char *clearnet = strchr(strchr(msg, ' ')+1, ' ')+1;

		char tmp[BUFSIZE];
		sprintf(tmp, "http://%s", onion);
		ini_write(lookup, "onions", tmp, clearnet);
		sprintf(tmp, "https://%s", onion);
		ini_write(lookup, "onions", tmp, clearnet);
		send_raw(s, 0, "PRIVMSG %s :sure\r\n", DEST); 
	}
}

void
scallions_init()
{
	mods_new("scallions", true);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
	lookup = ini_load("mods/scallions/lookup.ini");
	urls = ini_list_items(lookup, "onions");
}
