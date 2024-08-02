#include <stdio.h>
#include <string.h>
#include "urlutils.h"

char *
url_filter(char *haystack, char *res_url)
{
	/* check if theres any url at all */
	char *http = strstr(haystack, "http://");
	char *https = strstr(haystack, "https://");
	char *start = https ? https : http;
	if (!start) {
		return NULL;
	}

	/* filter out and clean url
	 * in possibly the worlds most terrible way */
	char url[2000] = {'\0'};
	strcpy(url, start);
	char *url_end = strpbrk(url, " ⟨⟩<>{}\\^\"\0");
	if (url_end) {
		url_end[0] = '\0';
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

	return res_url;
}

