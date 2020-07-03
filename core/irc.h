#ifndef IRC_H
#define IRC_H
/* required for types */
#include <openssl/ssl.h>
#include <stdbool.h>
#include "../ini_rw/ini_rw.h"

typedef struct {
	INI *globalconf;
	char index[256];
	char addr[256];
	char port[6];
    char nick[32];
	char pass[32];
    int *fd;
	SSL *sock;
	SSL_CTX *ctx;
	bool init;
} irc_conn;

void rejoin_channels(irc_conn *conn);
int init_conn(irc_conn *conn);
void destroy_conn(irc_conn *conn);
void join_chans(irc_conn *conn, char *chans);
void part_chans(irc_conn *conn, char *chans);
void send_raw(irc_conn *conn, char silent, char *msgformat, ...);

#define DEST chan[0] == '#' ? chan : user
/* bufsize is this oversized to deal with twitch's "500 characters" bs */
#define BUFSIZE 2000

#endif
