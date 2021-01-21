#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include "../../utils/curl.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "wa ", 3)) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1, strlen(strchr(msg, ' ')+1));
	sprintf(url, "http://api.wolframalpha.com/v1/result?appid=%s&i=%s",
			getenv("WOLFRAM_APPID"), req);

	/* configure curl request */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_fprivmsg("curl error: %s\r\n", curl_easy_strerror(r));
		curl_reset();
	} else {
		send_fprivmsg("%s\r\n", res.memory);
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	curl_free(req);
	free(res.memory);
}

void
wolfram_init()
{
	mods_new("wolfram", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
