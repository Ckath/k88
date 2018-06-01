#ifndef UTIL_H
#define UTIL_H

#include "intern.h"
#include "extern.h"

#define DEST channel[0] == '#' ? channel : sender

void handle_raw(int *sock, bool *reconnect, char *line);
void send_raw(int *sock, bool silent, char *msgformat, ...);
void join_chan(int *sock, char *chan);
void part_chan(int *sock, char *chan);
int strpos(char *match, char *str);
bool check_db(char *chandb);

#endif
