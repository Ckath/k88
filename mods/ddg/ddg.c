#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/irc.h"

static CURL *curl;
static bool curl_init = 0;

typedef struct chunk {
  char *memory;
  size_t size;
} chunk;

static size_t
write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	chunk *mem = (chunk *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

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
handle_cmdmsg(
		irc_conn *s, char *index, char *chan, char *user, char *msg, bool mod)
{
	if (strncmp(msg, "ddg ", 4)) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	if (!curl_init) {
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		curl_init = 1;
	}

	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1, strlen(strchr(msg, ' ')+1));
	sprintf(url, "https://api.duckduckgo.com/?q=%s&format=json&no_html=1", req);

	/* configure curl request */
	curl_easy_setopt(curl, CURLOPT_URL, url); 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res); 

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_raw(s, 0, "PRIVMSG %s :curl error: %s\r\n", DEST, curl_easy_strerror(r)); 
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
		send_raw(s, 0, "PRIVMSG %s :%s\r\n", DEST, response); 
	}

	/* cleanup */
	curl_free(req);
	free(res.memory);
}

void
ddg_init()
{
	mods_new("ddg", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
