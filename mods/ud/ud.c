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
	if (strncmp(msg, "ud ", 3)) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1, strlen(strchr(msg, ' ')+1));
	sprintf(url, "https://api.urbandictionary.com/v0/define?term=%s", req);

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
		char text[BUFSIZE];
		json_item(text, res.memory, "definition");
		json_item(url, res.memory, "permalink");
		if (url[0]) {
			strrplc(text, "\\n\\n", " ");
			strrplc(text, "\\r", "");
			strrplc(text, "\\", "");
			strrplc(text, "[", "");
			strrplc(text, "]", "");
			if (strlen(text) > 420) {
				strcpy(&text[417], "..");
			}
			send_privmsg("%s | %s", text, url);
		} else {
			send_privmsg("dunno");
		}
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	curl_free(req);
	if (res.memory) {
		free(res.memory);
	}
}

void
ud_init()
{
	mods_new("ud", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
