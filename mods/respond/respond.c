#define _GNU_SOURCE /* thats right strcasestr needs it */
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

#define AI_SEED "user: hi\\nmaidAI: hi\\nuser: h\\nmaidAI: hello!\\nuser: what are you?\\nmaidAI: I'm a maid belonging to ck\\nuser: you're stupid and useless\\nmaidAI: no u\\nuser: This is an excellent time for you to disappear\\nmaidAI: n-no u\\nuser:fuck off retard\\nmaidAI: you cannot stop me\\nuser: can you get me some coffeemaidAI: right away!\\n"

char hist[1444] = { "\0" };

static void
json_item(char *dest, char *json, char *item, char *end)
{
	char *start = strstr(json, item);
	if (!start) {
		dest[0] = '\0';
		return;
	}

	start += strlen(item)+1;
	strncpy(dest, start, BUFSIZE-1);
	char *e = strstr(dest, end);
	if (e) {
		e[0] = '\0';
	}
	return;
}

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* check for pings on nick,
	 * bit of a mess due to znc having nick in irnick */
	if (mi->cmd || (mi->conn->ircnick[0] != '\0' &&
		 !strcasestr(msg, mi->conn->ircnick)) ||
		 (mi->conn->ircnick[0] == '\0' &&
		 !strcasestr(msg, mi->conn->nick))) {
		return;
	}

	/* attempt to keep context history within reasonable size
	 * api favors smaller prompts */
	if (strlen(hist) > 1000) {
		char tmphist[1444];
		strcpy(tmphist, strstr(strstr(hist, "\\n")+2, "\\n")+2);
		strcpy(hist, tmphist);
	}

	/* tape in the latest conversation on user side */
	strcat(hist, "user: ");
	strcat(hist, msg+strlen(mi->conn->nick)+2);
	strcat(hist, "\\nmaidAI:");
	
	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char auth[BUFSIZE];
	char data[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1, strlen(strchr(msg, ' ')+1));
	sprintf(auth, "Authorization: Bearer %s",
			getenv("OPENAI_APPID"));
	sprintf(data, "{ \"model\": \"text-davinci-002\", \"prompt\": \"%s%s\", \"temperature\": 0.9, \"max_tokens\": 60, \"top_p\": 1.0, \"frequency_penalty\": 0.6, \"presence_penalty\": 0.8, \"stop\": [\"maidAI:\", \"user:\"] }",
		   AI_SEED, hist);

	/* configure curl request */
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, auth); 
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");
	curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/completions");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

retry:
	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_privmsg("curl error: %s", curl_easy_strerror(r));
		curl_reset();
	} else {
		char resb[BUFSIZE];
		char *r = resb;
		json_item(resb, res.memory, "text\":", "\",\"");
		if (!resb[0]) { /* brain damage */
			strcpy(hist, "");
			log_err("unusable response: %s\n", res.memory);
			send_privmsg("%s: response backend has failed me, try again",
					mi->user);
			return;
		}
		if (strlen(resb) > 420) {
			strcpy(resb+418, "..");
		}
		strrplc(resb, "\n", "");
		strrplc(resb, "\\n", "");
		if (resb[0] == ' ') {
			r++;
		}
		if (strstr("I'm sorry", r)) {
			goto retry;
		}
		send_privmsg("%s: %s", mi->user, r);
		strcat(hist, r);
		strcat(hist, "\\n");
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	curl_free(req);
	free(res.memory);
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (mi->mod && !strncmp(msg, "rclr", 4)) {
		strcpy(hist, "");
		send_privmsg("reset conversation buffer");
		return;
	}
}

void
respond_init()
{
	mods_new("respond", false);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
}
