#include "config.h" 
#include "socks.h"
#include "util.h"
#include "nodb.h"

static void init(int *sock);

static void
init(int *sock)
{
    puts("[ (!) ] initializing...");
    /* irc login */
    /* send_raw(sock, 1, "PRIVMSG nickserv :identify " PASS "\r\n"); */
    /* sleep(1); */

    send_raw(sock, 0, "CAP REQ :twitch.tv/membership\r\n");
    send_raw(sock, 0, "CAP REQ :twitch.tv/tags\r\n");
    send_raw(sock, 0, "CAP REQ :twitch.tv/commands\r\n");

    llist *chans = malloc(sizeof(llist));
    db_list(db_entry("db", "channels"), chans);

    llnode *runner = chans->head;
    while(runner != NULL) {
        if (strlen(runner->name) > 1) {
            printf("join channel '%s'\n", runner->name);
            join_chan(sock, runner->name);
        }
        runner = runner->next;
    }

    // memory leak but causes segfault otherwise
    /* free(chans); */

    /* join_chan(sock, CHAN); */
    /* join_chan(sock, CHAN); */
    /* join_chan(sock, channels, CHAN); */
    /* send_raw(sock, 0, "PRIVMSG " CHAN " :VoHiYo\r\n"); */
    /* join_chan(sock, "#arhdian"); */
}

int
main(int argc, char *argv[])
{
    for (;;) {
        /* make sure dirs exist */
        db_init("./db");
        db_init(db_entry("db", "channels"));

        /* open socket for connection */
        int sock;
        while(init_sock(&sock, SERV, PORT)) {
            puts("[ (!) ] reconnecting...");
            sleep(1);
        }

        /* create file handle from socket for reading */
        FILE *rsock;
        if ((rsock = fdopen(sock, "r")) == NULL) {
            fprintf(stderr, "[ !!! ] failed to socks\n");
            return 1;
        }

        puts("[ (!) ] attempting to identify...");
        send_raw(&sock, 1, "PASS " TOKEN "\r\n");
        send_raw(&sock, 0, "USER " NICK " 0 * :" NICK "\r\n");
        send_raw(&sock, 0, "NICK " NICK "\r\n");

        /* main loop */
        size_t len = 0;
        char *line_buf = NULL;
        bool init_flag = 1;
        ssize_t nread;
        bool reconnect = false;
        while (!reconnect && (nread = getline(&line_buf, &len, rsock)) != -1) {
            printf("[ <<< ] %s", line_buf);
            if (!strncmp(line_buf, ":tmi.twitch.tv RECONNECT", 24)) {
                puts("[ (!) ] caught a reconnect in loop WOW, trying to restart");
                break;
            }

            if (init_flag && !strncmp(line_buf, ":tmi.twitch.tv ", 15)) {
                init(&sock);
                init_flag = 0;
            }

            if (!strncmp(line_buf, "PING ", 5)) {
                puts("[ (!) ] pinged");
                line_buf[1] = 'O';
                send_raw(&sock, 0, line_buf);
            } else {
                handle_raw(&sock, &reconnect, line_buf);
            }
        }

        free(line_buf);
        close(sock);
        fclose(rsock);
        puts("[ (!) ] connection terminated");
    }

    return 0;
}
