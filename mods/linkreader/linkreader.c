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
find_title(char *dest, char *html)
{
	/* find valid */
	char start_match[33];
	strcpy(start_match, "<meta name=\"title\" content=\"");
	char *start = strcasestr(html, start_match);
	if (!start) {
		strcpy(start_match, "<title>");
		start = strcasestr(html, start_match);
	}
	if (!start) {
		dest[0] = '\0';
		return;
	}

	start += strlen(start_match);
	strncpy(dest, start, BUFSIZE-1);

	/* end title, lots of these exist sadly enough */
	char *end_match[] = {"</title>",
		"\">", "\"\\>", "\" >", "\" \\>", "\"/>", "\" />", NULL};
	for (int i = 0; end_match[i]; ++i) {
		char *title_end = strcasestr(dest, end_match[i]);
		if (title_end) {
			title_end[0] = '\0';
		}
	}

	/* title cleanup */
	strrplc(dest, "\n", "");
	strrplc(dest, "&amp;", "&");
	strrplc(dest, "&quot;", "\"");
	strrplc(dest, "&gt;", ">");
	strrplc(dest, "&lt;", "<");
	strrplc(dest, "&#039;", "'");
	strrplc(dest, "&apos;", "'");
	strrplc(dest, "&#x2019;", "'");
	strrplc(dest, "<br>", "");
	strrplc(dest, "&#x27;", "'");
	strrplc(dest, "&#39;", "'");
	strrplc(dest, "&#8211;", "-");
	strrplc(dest, "&mdash;", "-");
	strrplc(dest, "&ndash;", "-");
	strrplc(dest, "&#064;", "@");
	strrplc(dest, "&nbsp;", "");
	strrplc(dest, "\t", "");
	strrplc(dest, "  ", "");
	strunescape(dest);
}

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* check if theres any url at all */
	char *http = strstr(msg, "http://");
	char *https = strstr(msg, "https://");
	char *start = https ? https : http;
	if (!start) {
		return;
	}

	/* curl is stupid and breaks my sockets if I init it any sooner */
	setlocale(LC_ALL, ""); /* needed for encoding */
	curl_init();
	CURL *curl = curl_easy_init();

	/* filter out and clean url
	 * in possibly the worlds most terrible way */
	char url[BUFSIZE] = {'\0'};
	strcpy(url, start);
	char *url_end = strpbrk(url, " ⟨⟩<>{}\\^\"\0");
	if (url_end) {
		if (url_end[0] > 0) {
			url_end[0] = '\0';
		} else { /* workaround when ending in the middle of a wchar */
			while (*(++url_end) < 0);
			url_end = strpbrk(url_end,  " ⟨⟩<>{}\\^\"\0");
			if (url_end) {
				url_end[0] = '\0';
			}
		}
	}
	for(;;) {
		switch(url[strlen(url)-1]) {
			case ')':
				if (strchr(url, '(')) {
					break;
				}
			case ']':
				if (strchr(url, '[')) {
					break;
				}
			case '.':
			case ',':
			case ':':
			case ';':
			case '?':
			case '!':
			case '(':
			case '[':
			case '\'':
				url[strlen(url)-1] = '\0';
				continue;
		}
		break;
	}
	
	/* redirect entry point */
	int redirects = 0;
redirect:;
	if (redirects++ > 10) {
		send_privmsg("[ shitty site with >10 redirects ]");
		goto giveup;
	}

	/* twitter workaround */
	bool twitter_bs = strrplc(url, "twitter.com", "nitter.sethforprivacy.com");

	/* configure curl request */
	chunk res = { .memory = malloc(1), .size = 0 };
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "fuck/off");
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);

	/* handle result */
	CURLcode r = curl_easy_perform(curl);
	if (r != CURLE_OK) {
		log_err("curl error: %s\n", curl_easy_strerror(r));
	} else {
		long response_code = 0;
		r = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		if (response_code/100 == 3) { /* redirect */
			char *new_url;
			curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &new_url);
			strcpy(url, new_url);
			goto redirect;
		} else if (r != CURLE_OK) {
			goto giveup;
		}

		char title[BUFSIZE] = {'\0'};
		find_title(title, res.memory);

		if (title[0]) {
			if (twitter_bs) { /* hide the twitter -> nitter workaround */
				strrplc(title, " | Nitter - Seth For Privacy", "");
			} if (strlen(title) > 450) {
				strcpy(&title[447], "..");
			}
			send_privmsg("[ %s ]", title);
		}
	}
giveup:
	/* cleanup */
	curl_easy_cleanup(curl);
	if (res.memory) {
		free(res.memory);
	}
}

void
linkreader_init()
{
	mods_new("linkreader", false);
	mods_privmsg_handler(handle_privmsg);
}
