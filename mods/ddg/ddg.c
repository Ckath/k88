#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include "../../utils/curl.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
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
	if (strncmp(msg, "ddg ", 4)) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1, strlen(strchr(msg, ' ')+1));
	sprintf(url, "https://api.duckduckgo.com/?q=%s&format=json&no_html=1", req);

	/* configure curl request */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_fprivmsg("curl error: %s\r\n", curl_easy_strerror(r));
	} else {
		char response[BUFSIZE];
		char redirect[BUFSIZE];
		json_item(redirect, res.memory, "Redirect");
		if (redirect[0]) {
			sprintf(response, "find it yourself -> %s", redirect);
		} else {
			char url[BUFSIZE];
			char text[BUFSIZE];
			json_item(url, res.memory, "AbstractURL"),
			json_item(text, res.memory, "AbstractText");
			if (url[0]) {
				sprintf(response, text[0] ? "%s | %s" : "%s",
						url, text);
			} else {
				sprintf(response, "ddg's rubbish api did not return anything");
			}
			if (strlen(response) > 450) {
				strcpy(&response[447], "..");
			}
		}
		send_fprivmsg("%s\r\n", response);
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	curl_free(req);
	free(res.memory);
}

void
ddg_init()
{
	mods_new("ddg", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
