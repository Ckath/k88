#define _XOPEN_SOURCE /* strptime, this also breaks strdup for some reason */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"
#include "../../utils/strutils.h"

#define LOG_DIR "mods/lastseen/logs"

static INI *seen;
static unsigned scraped = 0;

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* TODO: possibly add this on rawmsg, though user lacks there */
	char lastseen[BUFSIZE];
	char luser[32] = { '\0' };
	strcpy(luser, mi->user);
	strtolower(luser);
	sprintf(lastseen, "%u %s <%s> %s", time(NULL), mi->chan, mi->user, msg);
	sini_write(seen, mi->conn->index, luser, lastseen);
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (!strncmp(msg, "seen ", 5)) {
		/* TODO: add a way to check other servers as well,
		 * or merge all into one entry and dont keep track which server it is */

		/* strip spaces */
		char name[256] = { '\0' };
		strcpy(name, strchr(msg, ' ')+1);
		if (strchr(name, ' ')) {
			strchr(name, ' ')[0] = '\0';
		}

		char lname[32] = { '\0' };
		strcpy(lname, name);
		strtolower(lname);
		char *lastseen = ini_read(seen, mi->conn->index, lname);
		if (lastseen) {
			char timestr[80];
			strncpy(timestr, lastseen, 79);
			lastseen = strchr(lastseen, ' ')+1;
			strchr(timestr, ' ')[0] = '\0';
			strtimef(timestr, time(NULL) - atol(timestr));
			send_privmsg("last msg: %s (%s ago)", lastseen, timestr);
		} else {
			send_privmsg("no data");
		}
	} else if (mi-> mod && !strncmp(msg, "seenscrape", 10)) {
		/* TODO: write this scraping logs */
		send_privmsg("todo :DDD");
		scraped = 0;
		DIR *dp = opendir(LOG_DIR);
		struct dirent *ep;
		while ((ep = readdir(dp))) {
			/* get rid of . and .. */
			if (ep->d_name[0] == '.' &&
					(ep->d_name[1] == '\0' ||
					(ep->d_name[1] == '.' && ep->d_name[2] == '\0'))) {
					continue;
			}

			/* setup file and date */
			char path[PATH_MAX];
			sprintf(path, LOG_DIR "/%s", ep->d_name);
			char date[11] = {'\0'}; /* from 20xx-xx-xx.log */
			strncpy(date, ep->d_name, 10);
			FILE *lf = fopen(path, "r");

			/* read in every line */
			char line_buf[BUFSIZE];
			/* [23:56:03] <mss> ah. */
			while (fgets(line_buf, sizeof(line_buf)-1, lf)) {
				/* check valid line, lazily */
				if (line_buf[0] != '[' ||
						!strchr(line_buf, '<') || !strchr(line_buf, '>')) {
					continue;
				}

				/* get timestamp */
				char timestamp[33] = {'\0'};
				sprintf(timestamp, "%s ", date);
				strncpy(timestamp+11, line_buf+1, 8);
				struct tm tm;
				strptime(timestamp, "%Y-%m-%d %H:%M:%S", &tm);
				time_t ts = mktime(&tm);

				/* get user */
				char user[32];
				strncpy(user, line_buf+12, 32);
				strchr(user, '>')[0] = '\0';

				/* check if worthwhile */
				char luser[32] = { '\0' };
				strcpy(luser, user);
				strtolower(luser);
				char *lastseen = ini_read(seen, mi->conn->index, luser);
				if (!lastseen) {
					goto add;
				}
				char timestr[80];
				strncpy(timestr, lastseen, 12);
				strchr(timestr, ' ')[0] = '\0';
				if (atol(timestr) > ts) {
					continue;
				}

add:;
				/* get and store msg */
				char msgg[BUFSIZE];
				char newseen[BUFSIZE];
				strcpy(msgg, strchr(strchr(line_buf, '>'), ' ')+1);
				/* TODO: let channel be argument */
				sprintf(newseen, "%u %s <%s> %s", ts, "#/g/punk", user, msgg);
				sini_write(seen, mi->conn->index, luser, newseen);
				log_info("updated %s '%s'", luser, newseen);
			}

			++scraped;
			log_info("scraped %u\n", scraped);
		}
		closedir(dp);
	}
}

void
lastseen_init()
{
	mods_new("lastseen", true);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
	seen = ini_load("mods/lastseen/seen.ini");
}
