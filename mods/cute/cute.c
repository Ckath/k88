#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "../../utils/strutils.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

/* yes, these are stolen from glitz,
 * they also use the same format to steal more in the future */
static char *cutes[] = {
	"(✿◠‿◠)っ~ ♥ {target}",
	"⊂◉‿◉つ ❤ {target}",
	"( ´・‿-) ~ ♥ {target}",
	"(っ⌒‿⌒)っ~ ♥ {target}",
	"ʕ´•ᴥ•`ʔσ” BEARHUG {target}",
	"(⊃｡•́‿•̀｡)⊃ U GONNA GET HUGGED {target}",
	"( ＾◡＾)っ~ ❤ {target}",
	"{target} (´ε｀ )♡",
	"{sender} ~(=^･ω･^)ヾ(^^ ) {target}",
	"{sender} (◎｀・ω・´)人(´・ω・｀*) {target}",
	"{sender} (*´・ω・)ノ(-ω-｀*) {target}",
	"{sender} (ɔ ˘⌣˘)˘⌣˘ c) {target}",
	"{sender} (◦˘ З(◦’ںˉ◦)♡ {target}"
};

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "cute", 4)) {
		return;
	}

	srand(time(NULL));
	char cute_pick[BUFSIZE] = {'\0'};
	strcpy(cute_pick, cutes[rand()%13]);

	char *target = strchr(msg, ' ');
	if (!target) {
		strrplc(cute_pick, "{sender}", "");
		strrplc(cute_pick, "{target}", "");
		send_fprivmsg("%s\r\n", cute_pick);
		return;
	}

	strrplc(cute_pick, "{sender}", mi->user);
	strrplc(cute_pick, "{target}", target+1);
	send_fprivmsg("%s\r\n", cute_pick);
}

void
cute_init()
{
	mods_new("cute", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
