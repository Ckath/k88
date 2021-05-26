#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include "../../utils/curl.h"
#include "../../utils/strutils.h"

/* required */
#include "../modtape.h"
#include "../../core/irc.h"
#include "../../core/log.h"
#include "../../core/modules.h"

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
parse_results(char *results)
{
	/* collect info, terribly */
	strunescape(results); /* just unescape the entirety, what a mess */
	char title[BUFSIZE] = { '\0' };
	char url[BUFSIZE] = { '\0' };
	char next_ep[BUFSIZE] = { '\0' };
	char eps[BUFSIZE] = { '\0' };
	char duration[BUFSIZE] = { '\0' };
	char description[BUFSIZE] = { '\0' };
	char score[BUFSIZE] = { '\0' };
	json_item(title, results, "userPreferred\":", "\"},\"");
	json_item(url, results, "siteUrl\":", "\",\"");
	json_item(next_ep, results, "nextAiringEpisode", "\",\"");
	json_item(eps, results, "episodes\"", ",\"");
	json_item(duration, results, "duration\"", ",\"");
	json_item(description, results, "description\":", "\",\"");

	/* TODO: think about if I care about showing the score, currently no */
	json_item(score, results, "averageScore\":", ",\"");
	if (!score[0]) {
		json_item(score, results, "meanScore\":", "}");
	}

	/* attempt at making the description okay on irc */
	strrplc(description, "<br><br>", " ");
	strrplc(description, "<br>\n<br>\n", " ");
	strrplc(description, "<br>", " ");
	strrplc(description, "\n", "");
	strrplc(description, "\\n", "");

	/* try to make some usable output from the info at hand */
	if (next_ep[0]) {
		char tua[BUFSIZE] = { '\0' };
		char ep[BUFSIZE] = { '\0' };
		json_item(tua, next_ep, "timeUntilAiring", ",\"");
		json_item(ep, next_ep, "episode\"", "}");
		unsigned seconds = strtoul(tua+1, NULL, 0);
		unsigned hours = seconds/60/60%24;
		unsigned days = seconds/60/60/24;

		if (days) {
			sprintf(results, "[%s x %smin] %s (ep%s airs in %d days, %d hours) %s | %s",
					eps, duration, title, ep, days, hours, url, description);
		} else if (hours) {
			sprintf(results, "[%s x %smin] %s (ep%s airs in %d hours) %s | %s",
					eps, duration, title, ep, hours, url, description);
		} else {
			sprintf(results, "[%s x %smin] %s (ep%s/%s airs in %d minutes) %s | %smin | %s",
					eps, duration, title, ep, seconds/60, url, description);
		}
	} else {
		char season[BUFSIZE] = { '\0' };
		char year[BUFSIZE] = { '\0' };
		json_item(season, results, "season\":", "\",");
		json_item(year, results, "seasonYear\"", ",\"");
		if (season[0]) {
			sprintf(results, "[%s x %smin] %s (%s %s) %s | %s",
					eps, duration, title, season, year, url, description);
		} else {
			json_item(year, results, "year\"", "}");
			sprintf(results, "[%s x %smin] %s (%s) %s | %s",
					eps, duration, title, year, url, description);
		}
	}

	if (strlen(results) > 420) {
		strcpy(results+418, "..");
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	bool anime_type = !strncmp(msg, "anime ", 6);
	if (!anime_type /*&& strncmp(msg, "mango ", 6)*/) { 
		/* TODO: consider if I care about manga search */
		return;
	}

	/* build url */
	char q[BUFSIZE];
	sprintf(q, anime_type ? "query={Media(search: \"%s\", type: ANIME) {title {userPreferred}siteUrl, description(asHtml:false)nextAiringEpisode {timeUntilAiring,episode}, season, seasonYear, endDate {year}, episodes, duration, averageScore, meanScore}}" :
			 "query={Media(search: \"%s\", type: MANGA) {title {userPreferred}siteUrl, description(asHtml:false)nextAiringEpisode {timeUntilAiring,episode}endDate {year, month, day}averageScore}}", strchr(msg, ' ')+1);

	/* curl is stupid and breaks my sockets if I init it any sooner */
	curl_init();
	CURL *curl = curl_easy_init();

	chunk res = { .memory = malloc(1), .size = 0 };

	/* configure curl request */
	curl_easy_setopt(curl, CURLOPT_URL, "https://graphql.anilist.co");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, q);

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		send_privmsg("curl error: %s", curl_easy_strerror(r));
	} else {
		char err[BUFSIZE];
		json_item(err, res.memory, "message", "\",\"");
		if (err[0]) {
			send_privmsg("%s", err+2);
		} else {
			parse_results(res.memory);
			send_privmsg("%s", res.memory);
		}

	}

	/* cleanup */
	curl_easy_cleanup(curl);
	free(res.memory);
}

void
anime_init()
{
	mods_new("anime", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
