#ifndef UTIL_H
#define UTIL_H

#include "intern.h"
#include "extern.h"

#define TOLOWER(variable) \
for (int i = 0; i < strlen(variable); ++i) \
    if (variable[i] >= 'A' && variable[i] <= 'Z') \
        variable[i] |= 1 << 5;
#define DEST channel[0] == '#' ? channel : user

unsigned long hash(char *str);
void start_handle_bux();
void handle_bux();
void handle_raw(int *sock, bool *reconnect, char *line);
void send_raw(int *sock, bool silent, char *msgformat, ...);
void join_chan(int *sock, char *chan);
void part_chan(int *sock, char *chan);
int strpos(char *match, char *str);
bool check_db(char *chandb);

#endif
