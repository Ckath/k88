#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static char *fortunes[] = {
	"13Reply hazy, try again",
	"7Excellent Luck",
	"7Good Luck",
	"3Average Luck",
	"3Bad Luck",
	"9Good news will come to you by mail",
	"10（　´_ゝ`）ﾌｰﾝ",
	"10ｷﾀ━━━━━━(ﾟ∀ﾟ)━━━━━━ !!!!",
	"11You will meet a dark handsome stranger",
	"12Better not tell you now",
	"6Outlook good",
	"6Very Bad Luck",
	"13Godly Luck"
};

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "fortune", 7)) {
		return;
	}
	srand(time(NULL));

	char *target = strchr(msg, ' ');
	send_privmsg("%s's fortune: %s", 
			target ? target+1 : mi->user, fortunes[rand()%13]);
}

void
fortune_init()
{
	mods_new("fortune", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
