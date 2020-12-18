#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include <stdbool.h>
#include "curl.h"

static bool curl_active = false;

size_t
curl_wrcb(void *contents, size_t size, size_t nmemb, void *userp)
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

void
curl_reset()
{
	curl_active = false;
	curl_global_cleanup();

	/* I cant figure out how to fix curl when its fucked here so restart bot instead */
	FILE *crashf = fopen("/tmp/k88_crash", "w+");
	fputs("restarted to unfuck curl", crashf);
	fclose(crashf);
	system("pkill k88");
}

void
curl_init()
{
	if (!curl_active) {
		curl_global_init(CURL_GLOBAL_ALL);
		curl_active = true;
	}
}
