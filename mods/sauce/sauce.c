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
parse_results(char *results)
{
	/* in case you're fixing index_id handling uncomment this */
	/* log_info("returned results: '%s'\n", results); */
	char score[10] = { '\0' };
	char index[10] = { '\0' };
	json_item(score, results, "similarity\":", "\",\"");
	json_item(index, results, "index_id\"", "\",\"");

	int index_id = strtoul(index, NULL, 0);
	int score_grade = strtoul(score, NULL, 0);
	score_grade = score_grade > 60 ? 3 : score_grade > 40 ? 7 : 4; 
	switch (index_id) { /* cursed switchcase of bad result type handling */
	case 21:; /* anime */
		char show[BUFSIZE] = { '\0' };
		char ep[BUFSIZE] = { '\0' };
		char time[BUFSIZE] = { '\0' };
		json_item(show, results, "source\":", "\",");
		json_item(ep, results, "part\":", "\",");
		json_item(time, results, "est_time\":", "\"");
		strunescape(show);
		strunescape(time);
		sprintf(results, "%d%s%%: %s of %s at %s",
				score_grade, score, ep, show, time);
		break;
	case 2:; /* game */
		char game[BUFSIZE] = { '\0' };
		char company[BUFSIZE] = { '\0' };
		json_item(game, results, "title\":", "\"");
		json_item(company, results, "company\":", "\"");
		strunescape(game);
		strunescape(company);
		sprintf(results, "%d%s%%: %s by %s",
				score_grade, score, game, company);
		break;
	case 38:; /* e-h doujin */
		char doujin[BUFSIZE] = { '\0' };
		char creator[BUFSIZE] = { '\0' };
		json_item(doujin, results, "source\":", "\"");
		json_item(creator, results, "creator\":", ",\"");
		strrplc(creator, "[", "");
		strunescape(creator);
		strunescape(doujin);
		sprintf(results, "%d%s%%: %s by %s",
				score_grade, score, doujin, creator);
		break;
		
	default:;
		char url[BUFSIZE] = { '\0' };
		char title[BUFSIZE] = { '\0' };
		json_item(url, results, "ext_urls\":[", "\"");
		json_item(title, results, "title\":", "\"");
		if (!url[0] && !title[0]) {
			sprintf(results, "%d%s%%: parser too shit to deal with result (id #%d)",
				score_grade, score, index_id);
			break;
		}
		strunescape(url);
		strunescape(title);
		sprintf(results, "%d%s%%: %s %s",
				score_grade, score, title, url);
		break;
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "sauce ", 6)) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	char *req = curl_easy_escape(curl, strchr(msg, ' ')+1, strlen(strchr(msg, ' ')+1));
	sprintf(url, "https://saucenao.com/search.php?api_key=%s&output_type=2&db=999&url=%s",
			getenv("SAUCE_APPID"), req);

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
		char results[BUFSIZE] = { '\0' };
		json_item(results, res.memory, "results\":[{", "}}");

		if (!results[0]) {
			json_item(results, res.memory, "message\":", "\"}}");
			send_privmsg("error: %s\n", results);
		} else {
			parse_results(results);
			send_privmsg("%s", results);
		}
	}

	/* cleanup */
	curl_easy_cleanup(curl);
	curl_free(req);
	free(res.memory);
}

void
sauce_init()
{
	mods_new("sauce", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
