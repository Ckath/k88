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
json_item(char *dest, char *json, char *item)
{
	char *start = strstr(json, item);
	if (!start || !strncmp(start+strlen(item), "\":\"\"", 4)) {
		dest[0] = '\0';
		return;
	}

	start += strlen(item)+3;
	strncpy(dest, start, BUFSIZE-1);
	strchr(dest, '"')[0] = '\0';
	return;
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "yt ", 3)) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1, strlen(strchr(msg, ' ')+1));
	sprintf(url, "https://www.youtube.com/results?search_query=%s", req);

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
		char redirect[BUFSIZE];
		char id[BUFSIZE];
		char title[BUFSIZE];
		json_item(id, res.memory, "videoId");
		json_item(title, res.memory, "\"text");
		if (id[0]) {
			send_privmsg("%s - https://www.youtube.com/watch?v=%s", title, id);
		} else {
			send_privmsg("couldnt find video");
		}
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	curl_free(req);
	free(res.memory);
}

void
yt_init()
{
	mods_new("yt", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
