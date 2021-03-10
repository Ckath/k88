#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "../../utils/strutils.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

/* yes, these are stolen from glitz,
 * they also use the same format to steal more in the future */
static char *target_cutes[] = {
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

static char *cutes[] = {
	"✿◕ ‿ ◕✿",
	"❀◕ ‿ ◕❀",
	"(✿◠‿◠)",
	"(◕‿◕✿) ",
	"( ｡◕‿◕｡)",
	"(◡‿◡✿)",
	"⊂◉‿◉つ ❤",
	"{ ◕ ◡ ◕}",
	"( ´・‿-) ~ ♥",
	"(っ⌒‿⌒)っ~ ♥",
	"ʕ´•ᴥ•`ʔσ”",
	"(･Θ･) caw",
	"(=^･ω･^)y＝",
	"ヽ(=^･ω･^=)丿",
	"~(=^･ω･^)ヾ(^^ )",
	"| (•□•) | (❍ᴥ❍ʋ)",
	"ϞϞ(๑⚈ ․̫ ⚈๑)∩",
	"ヾ(･ω･*)ﾉ",
	"~(=^･ω･^) nyaa~",
	"(◎｀・ω・´)人(´・ω・｀*)",
	"(*´・ω・)ノ(-ω-｀*)",
	"(❁´ω`❁)",
	"(＊◕ᴗ◕＊)",
	"{´◕ ◡ ◕｀}",
	"₍•͈ᴗ•͈₎",
	"(˘･ᴗ･˘)",
	"(ɔ ˘⌣˘)˘⌣˘ c)",
	"(⊃｡•́‿•̀｡)⊃",
	"(´ε｀ )♡",
	"(◦˘ З(◦’ںˉ◦)♡",
	"( ＾◡＾)っ~ ❤ ck",
	"╰(　´◔　ω　◔ `)╯",
	"(*･ω･)",
	"(∗•ω•∗)",
	"( ◐ω◐ )",
};

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "cute", 4)) {
		return;
	}

	srand(time(NULL));
	char cute_pick[BUFSIZE] = {'\0'};
	char *target = strchr(msg, ' ');

	/* prepare the cute, targeted or not */
	strcpy(cute_pick, target ? target_cutes[rand()%13] : cutes[rand()%35]);
	if (target) {
		strrplc(cute_pick, "{sender}", mi->user);
		strrplc(cute_pick, "{target}", target+1);
	}

	send_privmsg("%s", cute_pick);
}

void
cute_init()
{
	mods_new("cute", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
