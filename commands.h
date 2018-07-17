#ifndef COMMANDS_H
#define COMMANDS_H
#include "extern.h"
#include "irc.h"
#include "nodb.h"
#include "util.h"

/* permission levels */
#define BANNED 0
#define USER 1
#define MOD 2
#define BOT_MOD 3
#define STREAMER 4
#define BOT_OWNER 5

typedef struct {
    char trigger[20];
    char perm;
    void *func;
} command;

extern command *commands[24];

void init_commands();
void checkme(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void vanish(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void ball(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void checkme(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void vanish(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void ball(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void t(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void tnew(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void tedit(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void c(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void cnew(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void cedit(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void afk(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void whereis(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void mail(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void bux(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void give(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void gamble(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void timeout(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void join(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void part(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void prefix(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void enable(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void disable(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void checkdb(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void regreet(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
void setgreeting(irc_conn *conn, char *msg, char *user, char *channel, char *chandb);

#endif
