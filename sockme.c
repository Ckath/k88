#include "extern.h"
#include "intern.h"

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

    while(chans->head != NULL && 
            strlen(chans->head->name) > 1 &&
            chans->head->name[0] == '#') {
        join_chan(sock, chans->head->name);
        pop(chans, chans->head->name);
    }
    free(chans);
}

int
main(int argc, char *argv[])
{
    /* start alarm */
    alarm(BUX_INTERVAL);
	signal(SIGALRM, (void *) start_handle_bux);

    /* main loop for connecting/reconnecting everything */
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
        char line_buf[BUF_SIZE];
        bool init_flag = true;
        bool reconnect = false;
        while (!reconnect && fgets(line_buf, BUF_SIZE, rsock)) {
            printf("[ <<< ] %s", line_buf);
            if (!strncmp(line_buf, ":tmi.twitch.tv RECONNECT", 24)) {
                puts("[ (!) ] reconnecting...");
                break;
            }

            if (init_flag && !strncmp(line_buf, ":tmi.twitch.tv ", 15)) {
                init(&sock);
                init_flag = false;
            }

            if (!strncmp(line_buf, "PING ", 5)) {
                puts("[ (!) ] pinged");
                line_buf[1] = 'O';
                send_raw(&sock, 0, line_buf);
            } else {
                handle_raw(&sock, &reconnect, line_buf);
            }
        }

        close(sock);
        fclose(rsock);
        puts("[ (!) ] connection terminated");
    }

    return 0;
}
