#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../modules.h"
#include "../../irc.h"

static void
handle_privmsg(irc_conn *s, char *index, char *chan, char *user, char *msg)
{
	/* TODO: replace this with a 'replace' module
	 * which takes replacement pairs and stores them ini on command */
	char *match = strstr(msg, "://www.kohlchanvwpfx6hthoti5fvqsjxgcwm3tmddvpduph5fqntv5affzfqd.onion");
	if (match) {
		char tmp[2000] = {'\0'};
		strncpy(tmp, msg, match-msg);
		strcat(tmp, "s://kohlchan.net");
		strcat(tmp, match+69);
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
	}
	match = strstr(msg, "://kohlchanvwpfx6hthoti5fvqsjxgcwm3tmddvpduph5fqntv5affzfqd.onion");
	if (match) {
		char tmp[2000] = {'\0'};
		strncpy(tmp, msg, match-msg);
		strcat(tmp, "s://kohlchan.net");
		strcat(tmp, match+65);
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
	}
	match = strstr(msg, "://tew7tfz7dvv4tsom45z2wseql7kwfxnc77btftzssaskdw22oa5ckbqd.onion");
	if (match) {
		char tmp[2000] = {'\0'};
		strncpy(tmp, msg, match-msg);
		strcat(tmp, "s://anon.cafe");
		strcat(tmp, match+65);
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
	}
	match = strstr(msg, "https://pbs.twimg.com/media/");
	if (match) {
		char tmp[2000] = {'\0'};
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

void
politiksonions_init()
{
	mods_new("scallions", true);
	mods_privmsg_handler(handle_privmsg);
}
