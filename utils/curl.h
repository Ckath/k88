#ifndef CURL_UTIL
#define CURL_UTIL

typedef struct chunk {
  char *memory;
  size_t size;
} chunk;

size_t curl_wrcb(void *contents, size_t size, size_t nmemb, void *userp);
void curl_init();
void curl_reset();

#endif
