#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "core/irc.h"
#include "core/modules.h"
#include "ini_rw/ini_rw.h"

irc_conn *servers;
int servers_len = 0;

static void
parse_conf(INI *conf)
{
	char **server_list = ini_list_sections(conf);
	for (; server_list[servers_len]; ++servers_len) {
		servers = realloc(servers, sizeof(irc_conn)*(servers_len+1));
		servers[servers_len].globalconf = conf;
		strcpy(servers[servers_len].index, server_list[servers_len]);
		strcpy(servers[servers_len].addr, ini_read(conf,
					server_list[servers_len], "addr"));
		strcpy(servers[servers_len].port, ini_read(conf,
					server_list[servers_len], "port"));
		strcpy(servers[servers_len].nick, ini_read(conf,
					server_list[servers_len], "nick"));
		strcpy(servers[servers_len].pass, ini_read(conf,
					server_list[servers_len], "pass"));

		while (init_conn(&servers[servers_len])) {
			fprintf(stderr, "[ !!! ] failed to init server [%s] %s, retrying\n",
					server_list[servers_len], servers[servers_len].addr);
			destroy_conn(&servers[servers_len]);
		}
		printf("[ (!) ] loaded server [%s] %s\n",
				server_list[servers_len], servers[servers_len].addr);
	}
}

static void
sock_action(int signo, siginfo_t *info, void *context)
{
	for (int i = 0; i < servers_len; ++i) {
		if (info->si_fd == *servers[i].fd) {
			int n;
			char line_buf[BUFSIZE];
			char file[256];
			sprintf(file, "/tmp/%s", servers[i].index);

			/* TODO: split the irc connection part and cmd/msg handler
			 * into separate programs for easier restarts:
			 * - irc client program  buffers into file
			 * - parser program reads from file and handles messages
			 * - ??? parser sends message to client program, not sure how yet 
			 *
			 * for now it works out fine as is, for the most, 
			 * some stuff is missed with this abomination of a signal handler */

			/* buffer available data into file */
			FILE *file_buf = fopen(file, "w+");
			while((n = SSL_read(servers[i].sock, line_buf, sizeof(line_buf)-1)) > 0) {
				line_buf[n] = '\0';
				fputs(line_buf, file_buf);
			}

			/* handle all data */
			fseek(file_buf, 0, SEEK_SET);
			memset(line_buf, 0, sizeof(line_buf));
			while(fgets(line_buf, sizeof(line_buf)-1, file_buf)) {
				printf("[ <<< ] %s", line_buf);
				handle_modules(&servers[i], line_buf);
			}
			fclose(file_buf);
		}
	}
}

int
main(int argc, char *argv[])
{
	/* init signal handling */
    struct sigaction act;
    memset(&act, 0, sizeof(act));
	act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sock_action;
    sigaction(SIGIO, &act, 0);

	init_modules();
	INI *conf = ini_load("config.ini");
	parse_conf(conf);

	for(;;pause());

	/* should never get here */
	ini_free(conf);
	free(servers);
	return 0;
}
