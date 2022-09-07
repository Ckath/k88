#define _GNU_SOURCE /* thats right strcasestr needs it */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

#include "../markov/chains/src/chains.h" /* its markov time again */
#include "../../utils/strutils.h"

static struct markov_model *mm;
_Atomic static bool markov_loading = false;
static bool markov_loaded = false;
static FILE *pings;

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* bodged in lazy loading */
	if (!markov_loaded && !markov_loading) {
		markov_loading = true;
		log_info("response markov loading started\n");
		mm = mm_new();
		mm_learn_file(mm, "./mods/respond/pings.txt");
		markov_loaded = true;
		log_info("response markov loading finished\n");
		markov_loading = false;
		return;
	} if (markov_loading) {
		return;
	}

	/* check for pings on nick,
	 * bit of a mess due to znc having nick in irnick */
	char nick[32];
	strcpy(nick, mi->conn->ircnick[0] != '\0' ?
			mi->conn->ircnick : mi->conn->nick);
	if (mi->cmd || !strcasestr(msg, nick)) {
		return;
	}

	/* maybe needed? thanks lack of documentation */
	srand(time(NULL));

	char input[BUFSIZE]; 
	strcpy(input, msg);
	strcaserplc(input, nick, ""); 
	char *in = input;
	if (!strncmp(": ", input, 2) || !strncmp(", ", input, 2)) {
		in += 2;
	}
	fprintf(pings, "%s\n", in);
	fflush(pings);

	sleep(rand()%3+3);
	char response[MAX_LINE_LENGTH];
	if (mm_respond_and_learn(mm, in, response, 0)) {
		send_privmsg("%s: %s", mi->user, response);
	} else {
		send_privmsg("%s: brain dedded sorry", mi->user);
	}
}

void
respond_init()
{
	mods_new("respond", true);
	mods_privmsg_handler(handle_privmsg);
	pings = fopen("./mods/respond/pings.txt", "a+");
}
