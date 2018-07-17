#ifndef UTIL_H
#define UTIL_H

#include "extern.h"
#include "nodb.h"

#define TOLOWER(str) \
for (int i = 0; i < strlen(str); ++i) \
    if (str[i] >= 'A' && str[i] <= 'Z') \
        str[i] |= 1 << 5;
#define DEST channel[0] == '#' ? channel : user

unsigned long hash(char *str);
void start_handle_bux();
void handle_bux();
int strpos(char *match, char *str);
bool check_db(char *chandb);

#endif
