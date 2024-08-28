#ifndef STRUTILS
#define STRUTILS

char *strunescape(char *);
void str1rplc(char *haystack, char *needle, char *replace);
size_t strrplc(char *haystack, char *needle, char *replace);
size_t strcaserplc(char *haystack, char *needle, char *replace);
void strtolower(char *);
void strtoupper(char *);
char *strtimef(char *r, time_t t);

#define lowerdup(s, new) char *new = strdup(s); strtolower(new);
#define upperdup(s, new) char *new = strdup(s); strtoupper(new);
#endif 
