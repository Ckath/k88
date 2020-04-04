#include "extern.h"
#include "commands.h"
#include "irc.h"
#include "parse.h"
#include "util.h"

int
main(int argc, char *argv[])
{
    /* start alarm */
    alarm(BUX_INTERVAL);
	signal(SIGALRM, (void *) start_handle_bux);

    /* init commands */
    init_commands();

    /* main loop for connecting/reconnecting everything */
    for (;;) {
        irc_conn *bot = malloc(sizeof(irc_conn));
        strcpy(bot->nick, NICK);
        if(init_conn(bot, TOKEN)) {
            return 1;
        }

        /* main loop */
        char line_buf[BUF_SIZE];
        bool init_flag = true;
        bool reconnect = false;
        /* time shit */
        struct timeval tv;
        unsigned long utime = 0;
        while (fgets(line_buf, BUF_SIZE, bot->read_sock)) {
            gettimeofday(&tv, NULL);
            utime = 1000000 * tv.tv_sec + tv.tv_usec;

            printf("[ <<< ] %s", line_buf);
            if (!strncmp(line_buf, ":tmi.twitch.tv RECONNECT", 24)) {
                puts("[ (!) ] reconnecting...");
                break;
            }
            if (init_flag && !strncmp(line_buf, ":tmi.twitch.tv ", 15)) {
                rejoin_channels(bot);
                init_flag = false;
            }

            if (!strncmp(line_buf, "PING ", 5)) {
                puts("[ (!) ] pinged");
                line_buf[1] = 'O';
                send_raw(bot, 0, line_buf);

				if (init_flag) {
					rejoin_channels(bot);
					init_flag = false;
				}
            } else {
                handle_raw(bot, line_buf);
            }

            gettimeofday(&tv, NULL);
            utime = (1000000 * tv.tv_sec + tv.tv_usec) - utime;
            printf("^- took %luus\n", utime);
        }

        close(*bot->send_sock);
        fclose(bot->read_sock);
        destroy_conn(bot);
        puts("[ (!) ] connection terminated");
    }

    return 0;
}
