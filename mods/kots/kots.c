#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include "../../utils/strutils.h"
#include "../../utils/curl.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "kot", 3)) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	strcpy(url, "https://cataas.com/cat?json=true");

	/* configure curl request */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_privmsg("curl error: %s", curl_easy_strerror(r));
		curl_reset();
	} else if (!res.size) {
		send_privmsg("kot api error, didnt get kot");
	} else {
		char kot[BUFSIZE] = { '\0' };
		strcpy(kot, strstr(res.memory, "id")+5);
		strstr(kot, "\"")[0] = '\0';
		send_privmsg("https://cataas.com/cat/%s", kot);
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	free(res.memory);
}

void
kots_init()
{
	mods_new("kots", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
