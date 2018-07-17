#include "irc.h"

void
rejoin_channels(irc_conn *conn)
{
    puts("[ (!) ] initializing...");
    send_raw(conn, 0, "CAP REQ :twitch.tv/membership\r\n");
    send_raw(conn, 0, "CAP REQ :twitch.tv/tags\r\n");
    send_raw(conn, 0, "CAP REQ :twitch.tv/commands\r\n");

    puts("[ (!) ] rejoining channels...");
    llist *chans = malloc(sizeof(llist));
    db_list(db_entry("db", "channels"), chans);

    while(chans->head != NULL && 
            strlen(chans->head->name) > 1 &&
            chans->head->name[0] == '#') {
        join_chan(conn, chans->head->name);
        pop(chans, chans->head->name);
    }
    free(chans);
}

int
init_conn(irc_conn *conn, char token[100])
{
    /* make sure dirs exist */
    db_init("./db");
    db_init(db_entry("db", "channels"));

    /* create socket for connection */
    conn->send_sock = malloc(sizeof(int));
    while(init_sock(conn->send_sock, SERV, PORT)) {
        puts("[ (!) ] reconnecting...");
        sleep(1);
    }

    /* create file handle from socket for reading */
    if ((conn->read_sock = fdopen(*conn->send_sock, "r")) == NULL) {
        fprintf(stderr, "[ !!! ] failed to socks\n");
        return 1;
    }

    puts("[ (!) ] attempting to identify...");
    send_raw(conn, 1, "PASS %s\r\n", token);
    send_raw(conn, 0, "USER %s 0 * :%s\r\n", conn->nick, conn->nick);
    send_raw(conn, 0, "NICK %s\r\n", conn->nick);
    return 0;
}

void
destroy_conn(irc_conn *conn)
{
    free(conn->send_sock);
    free(conn);
}

void
join_chan(irc_conn *conn, char *chan)
{
    /* check if db is alright */
    char *chandb = db_getdb("db", chan);
    db_init(chandb);
    check_db(chandb);
    free(chandb);

    db_mkitem(db_entry("db", "channels", chan));
    send_raw(conn, 0, "JOIN %s\r\n", chan);
}

void
part_chan(irc_conn *conn, char *chan)
{
    db_del(db_entry("db", "channels", chan));
    send_raw(conn, 0, "PART %s\r\n", chan);
}

void
send_raw(irc_conn *conn, bool silent, char *msgformat, ...)
{
    char buf[BUF_SIZE];
    va_list args;
    va_start(args, msgformat);
    vsnprintf(buf, BUF_SIZE, msgformat, args);
    va_end(args);

    if (send(*conn->send_sock, buf, strlen(buf), 0) < 0 && !silent) {
        fprintf(stderr, "[ !!! ] failed to send: '%s'", buf);
    } else if (!silent) {
        printf("[ >>> ] %s", buf);
    }
}
