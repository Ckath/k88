#ifndef STRUTILS
#define STRUTILS

char *strunescape(char *);
size_t strrplc(char *haystack, char *needle, char *replace);
void strtolower(char *);
void strtoupper(char *);

#define lowerdup(s, new) char *new = strdup(s); strtolower(new);
#define upperdup(s, new) char *new = strdup(s); strtoupper(new);
#endif 
