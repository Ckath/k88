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

#define DUCK_STATUS "https://duckduckgo.com/duckchat/v1/status"
#define DUCK_CHAT "https://duckduckgo.com/duckchat/v1/chat"
#define FAKE_AGENT "Mozilla/5.0 (Windows NT 10.0; rv:122.0) Gecko/20100101 Firefox/122.0"

static char duck_key[50] = {'\0'};
static pthread_mutex_t ai_lock = PTHREAD_MUTEX_INITIALIZER;

static size_t
duck_hcb(char *buffer, size_t size, size_t nitems, void *userdata)
{
	/* capture key from x-vqd-4 */
	if (!strncmp(buffer, "x-vqd-4:", 8) &&
			strstr(buffer, "4-")) {
		char *key = (char *) userdata;
		strncpy(key, strstr(buffer, "4-"), 50);
		strchr(key, '\r')[0] = '\0';
	}

	return nitems * size;
}

size_t
duck_wrcb(void *contents, size_t size, size_t nmemb, void *userp)
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
json_filter(char *dest, char *json, char *start, char *end)
{
	char *s = strstr(json, start);
	if (!s) {
		dest[0] = '\0';
		return;
	}

	s += strlen(start);
	strncpy(dest, s, BUFSIZE-1);
	char *e = strstr(dest, end);
	if (e) {
		e[0] = '\0';
	}
	return;
}

static void
duck_parse(chunk *res, char *response)
{ 
	char *jsonptr = res->memory;
	char chunk[BUFSIZE] = {'\0'};
	do {
		/* filter each message chunk */
		json_filter(chunk, jsonptr, "ssage\":\"", "\",\"crea");
		if (!chunk[0]) {
			jsonptr = strchr(jsonptr, '}')+1; 
			continue;
		}

		/* add while valid */
		strcat(response, chunk);
		jsonptr = strchr(jsonptr, '}')+1; 
	} while(jsonptr && strlen(jsonptr) > 100 && strlen(response) < 440);

	/* keep within length */
	if (strlen(response) > 439) {
		strcpy(&response[437], "..");
	}
}

static void
duck_send(CURL *curl, chunk *res, char *prompt)
{
	struct curl_slist *slist1 = NULL;
	char vqd[100];
	char query[BUFSIZE];
	sprintf(vqd, "x-vqd-4: %s", duck_key);
	sprintf(query, "{\"model\":\"mistralai/Mixtral-8x7B-Instruct-v0.1\","
			"\"messages\":[{\"role\":\"user\",\"content\":"
			"\"Respond as concisely as possible. "
			"Never use more than 400 characters. %s\"}]}", prompt);

	slist1 = curl_slist_append(slist1, "User-Agent: Mozilla/5.0 (Windows NT 10.0; rv:122.0) Gecko/20100101 Firefox/122.0");
	slist1 = curl_slist_append(slist1, "Accept: text/event-stream");
	slist1 = curl_slist_append(slist1, "Referer: https://duckduckgo.com/");
	slist1 = curl_slist_append(slist1, "Content-Type: application/json");
	slist1 = curl_slist_append(slist1, vqd);
	slist1 = curl_slist_append(slist1, "Origin: https://duckduckgo.com");

	curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
	curl_easy_setopt(curl, CURLOPT_URL, DUCK_CHAT);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)strlen(query));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, FAKE_AGENT);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, duck_wrcb);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, duck_hcb);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, duck_key);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);
	curl_easy_perform(curl);

	/* if invalid response wipe key, retry later */
	if (res->memory && !strstr(res->memory, "data: ")) {
		duck_key[0] = '\0';
		return;
	}
}

static void
duck_init(CURL *curl)
{
	chunk res = { .memory = malloc(1), .size = 0 };
	struct curl_slist *slist1 = NULL;
	slist1 = curl_slist_append(slist1, "x-vqd-accept: 1");
	slist1 = curl_slist_append(slist1, "User-Agent: " FAKE_AGENT);
	slist1 = curl_slist_append(slist1, "Referer: https://duckduckgo.com/");

	curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
	curl_easy_setopt(curl, CURLOPT_URL, DUCK_STATUS);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, FAKE_AGENT);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, duck_wrcb);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, duck_hcb);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, duck_key);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_perform(curl);

	if (res.memory) {
		free(res.memory);
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "QA", 2)) {
		return;
	}

	/* curl really hates doing this multiple times */
	pthread_mutex_lock(&ai_lock);

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();
	chunk res = { .memory = malloc(1), .size = 0 };

	char *prompt = strchr(msg, ' ') ? strchr(msg, ' ')+1 : "";
	char response[BUFSIZE] = {'\0'};

	int i = 0;
	for (; i < 5; ++i) {
		/* key never obtained or invalid response */
		if (!duck_key[0]) {
			duck_init(curl);
			log_info("duck_key: '%s'\n", duck_key);
			if (!duck_key[0]) {
				sleep(5);
				continue;
			}
		}

		/* send, check, parse */
		log_info("duck_key before chat: '%s'\n", duck_key);
		duck_send(curl, &res, prompt);
		if (!strstr(res.memory, "data: ")) { /* fucked response */
			sleep(5);
			continue;
		}

		/* success */
		duck_parse(&res, response);

		/* it shouldnt be the case that the key invalides but it does,
		 * spamming chat endpoint with invalid keys causes being blocked */
		/* TODO: look into for context awareness */
		duck_key[0] = '\0';

		break;
	}
	if (res.memory) {
		free(res.memory);
	}

	if (i > 4) {
		send_privmsg("'api' handling too shitty, probably blocked again");
	} else {
		strrplc(response, "\\n", " "); 
		strrplc(response, "\\\"", "\""); 
		send_privmsg("%s", response+1);
	}
	pthread_mutex_unlock(&ai_lock);
}

void
ai_init()
{
	mods_new("ai", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
