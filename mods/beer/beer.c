#define _GNU_SOURCE /* strcasestr */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include <locale.h>
#include "../../utils/curl.h"
#include "../../utils/strutils.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static void
filter_beer(char *id, char *name, char *abv, char *ibu, char *html)
{
	char *beerptr = strstr(html, "beer-item");
	if (!beerptr) {
		id = NULL;
		return;
	}

	/* find id */
	strncpy(id, strstr(beerptr, "/beer/"), 22);
	strchr(id, '"')[0] = '\0';
	/* find name */
	strncpy(name, strstr(strstr(strstr(beerptr, "beer-details"), "/b/"), "\">"), 255);
	strchr(name, '<')[0] = '\0';
	strrplc(name, "\">", "");
	strrplc(name, "\t", "");
	/* find abv */
	strncpy(abv, strstr(beerptr, "\"abv\">\n"), 33);
	strrplc(abv, "\"abv\">\n", "");
	strchr(abv, '\n')[0] = '\0';
	strrplc(abv, "\t", "");
	/* find ibu */
	strncpy(ibu, strstr(beerptr, "\"ibu\">\n"), 33);
	strrplc(ibu, "\"ibu\">\n", "");
	if (strchr(ibu, '<')) {
		strchr(ibu, '<')[0] = '\0';
	}
	strrplc(ibu, "</p>", "");
	strrplc(ibu, "\t", "");
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if ((strncmp(msg, "beer ", 5) && strncmp(msg, "ba ", 3))
			|| !strchr(msg, ' ')) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	setlocale(LC_ALL, ""); /* needed for encoding */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char url[BUFSIZE] = "https://untappd.com/search?q=";
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1, strlen(strchr(msg, ' ')+1));
	strcat(url, req);


	/* configure curl request */
	chunk res = { .memory = malloc(1), .size = 0 };
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		log_err("curl error: %s\n", curl_easy_strerror(r));
	} else {
		char id[BUFSIZE] = {'\0'};
		char name[BUFSIZE] = {'\0'};
		char abv[BUFSIZE] = {'\0'};
		char ibu[BUFSIZE] = {'\0'};
		filter_beer(id, name, abv, ibu, res.memory);

		if (!id[0]) {
			send_privmsg("404 no beers found :(", name, id, abv, ibu);
		} else {
			send_privmsg("%s https://untappd.com%s | %s, %s", name, id, abv, ibu);
		}
	}

	curl_easy_cleanup(curl);
	if (res.memory) {
		free(res.memory);
	}
}

void
beer_init()
{
	mods_new("beer", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
