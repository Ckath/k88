#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include "../../utils/curl.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

#include "../../utils/strutils.h"

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strstr(msg, ".") || strstr(msg, "/")) {
		return; /* no escaping */
	} 
	
	/* open file if its there */
	char quote_path[BUFSIZE];
	strcpy(quote_path, "mods/quotes/quotes/");
	strcat(quote_path, msg);
	if (access(quote_path, F_OK)) {
		return; /* no quote file found */
	}

	/* load in file */
	FILE *f = fopen(quote_path, "r");
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char data[size+1];
	fread(data, 1, size, f);
	data[size] = '\0';

	/* count lines */
	char *r = data;
	char *line_end;
	long lines = 0;
	while ((line_end = strchr(r, '\n'))) {
		lines++;
		r = line_end+1;
	}

	/* pick random line and get it */
	srand(time(NULL));
	long pick = (rand()%lines)+1;
	log_info("lines: %ld, pick %ld\n", lines, pick);
	r = data;
	lines = 0;
	fseek(f, 0, SEEK_SET);
	while ((line_end = strchr(r, '\n'))) {
		if (++lines == pick) {
			/* TODO: parse shit here like in triggers */
			char quote[BUFSIZE];
			strncpy(quote, r, BUFSIZE-1);
			strchr(quote, '\n')[0] = '\0';
			send_privmsg("%s", quote);
			return;
		}
		r = line_end+1;
	}
}

void
quotes_init()
{
	mods_new("quotes", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
