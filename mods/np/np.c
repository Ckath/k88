#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include "../../utils/curl.h"
#include "../../utils/strutils.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static INI *accs;

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "np", 2)) {
		return;
	}

	char *cmd = strchr(msg, ' ');
	if (cmd) {
		cmd++;
	}

	/* link account */
	if (cmd && !strncmp(cmd, "link", 4)) {
		char *acc = strchr(strstr(cmd, "link"), ' ');
		if (acc) {
			acc++;
		} else {
			send_privmsg("invalid");
			return;
		}

		sini_write(accs, "accounts", mi->user, acc);
		send_privmsg("linked lastfm account %s", acc);
		return;
	}

	/* account linked check */
	char *lf_acc = ini_read(accs, "accounts", mi->user);
	if (!lf_acc) {
		send_privmsg("no lastfm account linked, run %snp link lastfmacc",
				mods_get_prefix(mi->conn, mi->index));
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	sprintf(url, "https://ws.audioscrobbler.com/2.0/?"
			"method=user.getrecenttracks&user=%s&limit=1&api_key=%s",
			lf_acc, getenv("LASTFM_APPID"));

	/* configure curl request */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_privmsg("curl error: %s", curl_easy_strerror(r));
		curl_reset();
	} else {
		char artist[200];
		char song[255];

		/* start, check if nowplaying or lastplaying */
		char *p = strstr(res.memory, "<track");
		bool np = strstr(p, "nowplaying=");

		/* artist */
		p = strchr(strstr(p, "<artist"), '>')+1;
		strncpy(artist, p, 199);
		strstr(artist, "</artist>")[0] = '\0';

		/* song */
		p = strstr(p, "<name>")+6;
		strncpy(song, p, 254);
		strstr(song, "</name>")[0] = '\0';

		send_privmsg("%s: %s - %s", np ? "now playing" : "last played",
				artist, song);
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	free(res.memory);
}

void
np_init()
{
    accs = ini_load("mods/np/accounts.ini");

	mods_new("np", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
