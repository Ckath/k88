#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>

#include "core/irc.h"
#include "core/modules.h"
#include "ini_rw/ini_rw.h"

irc_conn *servers;
size_t nservers = 0;

static void
parse_conf(INI *conf)
{
	char **server_list = ini_list_sections(conf);
	for (; server_list[nservers]; ++nservers) {
		servers = realloc(servers, sizeof(irc_conn)*(nservers+1));
		servers[nservers].globalconf = conf;
		strcpy(servers[nservers].index, server_list[nservers]);
		strcpy(servers[nservers].addr, ini_read(conf,
					server_list[nservers], "addr"));
		strcpy(servers[nservers].port, ini_read(conf,
					server_list[nservers], "port"));
		strcpy(servers[nservers].nick, ini_read(conf,
					server_list[nservers], "nick"));
		char *pass = ini_read(conf, server_list[nservers], "pass");
		if (pass) {
			servers[nservers].pass = malloc(strlen(pass)+1);
			strcpy(servers[nservers].pass, pass);
		} else {
			servers[nservers].pass = NULL;
		}

		while (init_conn(&servers[nservers])) {
			fprintf(stderr, "[ !!! ] failed to init server [%s] %s, retrying\n",
					server_list[nservers], servers[nservers].addr);
			destroy_conn(&servers[nservers]);
		}
		printf("[ (!) ] loaded server [%s] %s\n",
				server_list[nservers], servers[nservers].addr);
	}
}

static void
sock_action(int signo, siginfo_t *info, void *context)
{
	for (int i = 0; i < nservers; ++i) {
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
				mod_arg *args = malloc(sizeof(mod_arg));
				args->conn = &servers[i];
				strcpy(args->line, line_buf);
				pthread_create(&args->thr, NULL, (void *)handle_modules, args);
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

	/* loop calling timed modules, will get interrupted to handle others */
	for (;;) {
		timed_arg *args = malloc(sizeof(timed_arg));
		args->conn = servers;
		args->n = nservers;
		args->t = time(NULL);
		pthread_create(&args->thr, NULL, (void *)timed_modules, args);
		sleep(1);
	}

	/* should never get here */
	ini_free(conf);
	free(servers);
	return 0;
}
