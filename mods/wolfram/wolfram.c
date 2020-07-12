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
handle_cmdmsg(
		irc_conn *s, char *index, char *chan, char *user, char *msg, bool mod)
{
	if (strncmp(msg, "wa ", 3)) {
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
	sprintf(url, "http://api.wolframalpha.com/v1/result?appid=%s&i=%s",
			getenv("WOLFRAM_APPID"), req);

	/* configure curl request */
	curl_easy_setopt(curl, CURLOPT_URL, url); 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res); 

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_fprivmsg("curl error: %s\r\n", curl_easy_strerror(r)); 
	} else {
		send_fprivmsg("%s\r\n", res.memory); 
	}

	/* cleanup */
	curl_free(req);
	free(res.memory);
}

void
wolfram_init()
{
	mods_new("wolfram", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
