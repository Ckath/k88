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
	char *kohl_match = strstr(msg, "kohlchanvwpfx6hthoti5fvqsjxgcwm3tmddvpduph5fqntv5affzfqd.onion");
	if (kohl_match) {
		char tmp[2000];
		strncpy(tmp, msg, kohl_match-msg);
		strcat(tmp, "kohlchan.net");
		strcat(tmp, kohl_match+62);
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, tmp); 
	}
}

void
kohlchanonions_init()
{
	mods_new("kohlchanonions", true);
	mods_privmsg_handler(handle_privmsg);
}
