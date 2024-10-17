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
	} if (!strncmp(dest, ":null", 5) ||
			!strncmp(dest, "ull,", 4) ||
			!strncmp(dest, "null", 4)) {
		dest[0] = '\0';
	}
	return;
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "crypto ", 7)) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url and auth */
	char url[BUFSIZE];
	char auth[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1,
			strlen(strchr(msg, ' ')+1));
	sprintf(url, "https://pro-api.coinmarketcap.com/v2/cryptocurrency/quotes/"
			"latest?symbol=%s&convert=USD", req);
	sprintf(auth, "X-CMC_PRO_API_KEY: %s", getenv("CRYPTO_APPID"));

	/* configure curl request */
	struct curl_slist *slist = NULL;
	slist = curl_slist_append(slist, auth);
	slist = curl_slist_append(slist, "Accept: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_privmsg("curl error: %s", curl_easy_strerror(r));
		curl_reset();
	} else {
		/* check valid response */
		if (strstr(res.memory, "symbol")) {
			/* gather info */
			char symbol[BUFSIZE] = {'\0'};
			char name[BUFSIZE] = {'\0'};
			char price[BUFSIZE] = {'\0'};
			char hperc[BUFSIZE] = {'\0'};
			char hhperc[BUFSIZE] = {'\0'};
			char dperc[BUFSIZE] = {'\0'};
			char ddperc[BUFSIZE] = {'\0'};
			char wperc[BUFSIZE] = {'\0'};
			char wwperc[BUFSIZE] = {'\0'};
			json_item(symbol, res.memory, "symbol\":", "\",\"");
			json_item(name, res.memory, "name\":", "\",\"");
			json_item(price, res.memory, "price\"", ",\"");
			json_item(hperc, res.memory, "percent_change_1h\"", ",\"");
			json_item(dperc, res.memory, "percent_change_24h\"", ",\"");
			json_item(wperc, res.memory, "percent_change_7d\"", ",\"");

			/* colorize */
			sprintf(hhperc, "%s%s", hperc[0] == '-' ? "4 " : "3 +", hperc);
			strcat(hhperc, "");
			sprintf(ddperc, "%s%s", dperc[0] == '-' ? "4 " : "3 +", dperc);
			strcat(ddperc, "");
			sprintf(wwperc, "%s%s", wperc[0] == '-' ? "4 " : "3 +", wperc);
			strcat(wwperc, "");

			send_privmsg("1 %s (%s) = %s USD | hourly%s%% | daily%s%% | weekly%s%%",
					symbol, name, price, hhperc, ddperc, wwperc);
		} else {
			send_privmsg("symbol not found");
		}
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	curl_free(req);
	free(res.memory);
}

void
crypto_init()
{
	mods_new("crypto", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
