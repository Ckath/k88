#ifndef IRC_H
#define IRC_H

#include "extern.h"
#include "nodb.h"
#include "util.h"
#include "socks.h"
#include "config.h"

typedef struct {
    char nick[32];
    int *send_sock;
    FILE *read_sock;
} irc_conn;

void rejoin_channels(irc_conn *conn);
int init_conn(irc_conn *conn, char token[100]);
void destroy_conn(irc_conn *conn);
void join_chan(irc_conn *conn, char *chan);
void part_chan(irc_conn *conn, char *chan);
void send_raw(irc_conn *conn, bool silent, char *msgformat, ...);

#endif
