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
	char *kohl_match = strstr(msg, "://www.kohlchanvwpfx6hthoti5fvqsjxgcwm3tmddvpduph5fqntv5affzfqd.onion");
	if (kohl_match) {
		char tmp[2000] = {'\0'};
		strncpy(tmp, msg, kohl_match-msg);
		strcat(tmp, "s://kohlchan.net");
		strcat(tmp, kohl_match+69);
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
	}
	kohl_match = strstr(msg, "://kohlchanvwpfx6hthoti5fvqsjxgcwm3tmddvpduph5fqntv5affzfqd.onion");
	if (kohl_match) {
		char tmp[2000] = {'\0'};
		strncpy(tmp, msg, kohl_match-msg);
		strcat(tmp, "s://kohlchan.net");
		strcat(tmp, kohl_match+65);
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
	}
	kohl_match = strstr(msg, "://tew7tfz7dvv4tsom45z2wseql7kwfxnc77btftzssaskdw22oa5ckbqd.onion");
	if (kohl_match) {
		char tmp[2000] = {'\0'};
		strncpy(tmp, msg, kohl_match-msg);
		strcat(tmp, "s://anon.cafe");
		strcat(tmp, kohl_match+65);
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
	}
}

void
kohlchanonions_init()
{
	mods_new("kohlchanonions", true);
	mods_privmsg_handler(handle_privmsg);
}
