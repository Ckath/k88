#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

#include "../../ini_rw/ini_rw.h"
#include "../../utils/strutils.h"

static INI *lookup;
static char **urls;

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* copy msg */
	bool url_fixed = false;
	char buf[BUFSIZE] = {'\0'};
	strcpy(buf, msg);

	/* replace all known onions */
	for (int i = 0; urls[i]; ++i) {
		if (strrplc(buf, urls[i], ini_read(lookup, "onions", urls[i]))) {
			url_fixed = true;
		}
	}

	/* if an onion remains, mirror it to tmphost, only works for 1 */
	if ((strstr(buf, "http://") || strstr(buf, "https://"))
			&& strstr(buf, ".onion")) {
		/* filter url out */
		char *onion = strstr(buf, ".onion");
		while(--onion != buf && strncmp("http", onion, 4));
		char url[BUFSIZE] = {'\0'};
		strcpy(url, onion);
		char *url_end = strpbrk(url, " ⟨⟩<>{}\\^\"\0,:;?!(['|&");
		if (url_end) {
			url_end[0] = '\0';
		}

		/* awful check for direct file link (strlen extension) */
		if (strlen(url)-(strrchr(url, '.')-url) < 5) {
			char cmd[BUFSIZE] = {'\0'};
			sprintf(cmd, "torsocks curl -so /tmp/$(basename '%s') '%s'" \
					" && ~/tmpupload.sh /tmp/$(basename '%s')",
					url, url, url);
			FILE *cp = popen(cmd, "r");
			char tmpurl[BUFSIZE] = {'\0'};
			fgets(tmpurl, BUFSIZE-1, cp);
			pclose(cp);
			strrplc(tmpurl, "\n", "");
			strrplc(buf, url, tmpurl);
			url_fixed = true;
		}
	}

	/* send if altered */
	if (url_fixed) {
		send_fprivmsg("%s\r\n", buf);
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	/* admin only */
	if (!mi->mod) {
		return;
	}

	if (!strncmp(msg, "onion ", 6)) {
		char onion[BUFSIZE];
		strcpy(onion, strchr(msg, ' ')+1);
		strchr(onion, ' ')[0] = '\0';

		char *clearnet = strchr(strchr(msg, ' ')+1, ' ')+1;

		char tmp[BUFSIZE];
		sprintf(tmp, "http://%s", onion);
		ini_write(lookup, "onions", tmp, clearnet);
		sprintf(tmp, "https://%s", onion);
		ini_write(lookup, "onions", tmp, clearnet);
		urls = ini_list_items(lookup, "onions");
		send_privmsg("sure\r\n");
	}
}

void
scallions_init()
{
	mods_new("scallions", true);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
	lookup = ini_load("mods/scallions/lookup.ini");
	urls = ini_list_items(lookup, "onions");
}
