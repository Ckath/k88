#include <string.h>
#include "../core/irc.h" /* for BUFSIZE */
#include "strutils.h"

size_t
strrplc(char *haystack, char *needle, char *replace)
{	
	/* replace occurrences and return fixed string */
	char *match;
	size_t matches = 0;
	while ((match = strstr(haystack, needle))) {
		char tmp[BUFSIZE] = {'\0'};
		strncpy(tmp, haystack, match-haystack);
		strcat(tmp, replace);
		strcat(tmp, match+strlen(needle));
		strcpy(haystack, tmp);
		matches++;
	}
	return matches;
}

char *
strlower(char *str)
{
	for (int i = 0; str[i]; ++i) {
		if (str[i] >= 'A' && str[i] <= 'Z') {
			str[i] += 32;
		}
	}
	return str;
}
