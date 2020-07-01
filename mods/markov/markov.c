#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

#include "chains/src/chains.h"

struct markov_model *mm;
FILE *learned;

static void
markov_learnall(char *dir)
{
	DIR *dp = opendir(dir);
	struct dirent *ep;
	while ((ep = readdir(dp))) {
		/* get rid of . and .. */
		if (ep->d_name[0] == '.' &&
				(ep->d_name[1] == '\0' ||
				(ep->d_name[1] == '.' && ep->d_name[2] == '\0'))) {
				continue;
		}
		char path[PATH_MAX];
		sprintf(path, "%s/%s", dir, ep->d_name);
		mm_learn_file(mm, path);
	}
	closedir(dp);
}

static void
handle_privmsg(irc_conn *s, char *index, char *chan, char *user, char *msg)
{
	/* dont log own commands to markov learncache */
	char *prefix = mods_get_prefix(s, index);
	if (!strncmp(prefix, msg, strlen(prefix))) {
		return;
	}

	mm_learn_sentence(mm, msg);
	fprintf(learned, "%s\n", msg);
	fflush(learned);
}

static void
handle_cmdmsg(
		irc_conn *s, char *index, char *chan, char *user, char *msg, bool mod)
{
	if (strncmp(msg, "markov", 6)) {
		return;
	}

	/* maybe needed? thanks lack of documentation */
	srand(time(NULL)); 

	char input[MAX_LINE_LENGTH];
	char *arg = strchr(msg, ' ');
	strcpy(input, arg ? arg : "");

	char response[MAX_LINE_LENGTH];
	if (mm_respond_and_learn(mm, input, response, 0)) {
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, response);
	} else {
		send_raw(s, 0, "PRIVMSG %s :failed to markov\r\n", DEST);
	}
}

void
markov_init()
{
	mods_new("markov", false);
	mods_cmdmsg_handler(handle_cmdmsg);
	mods_privmsg_handler(handle_privmsg);
	mm = mm_new();
	markov_learnall("./mods/markov/learndata");
	learned = fopen("./mods/markov/learndata/learned.txt", "a+");
}
