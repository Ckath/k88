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
#include "core/log.h"
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
		servers[nservers].twitch = ini_read(conf, server_list[nservers], "twitch");

		servers[nservers].reconns = 0;
		while (init_conn(&servers[nservers])) {
			log_err("failed to init server [%s] %s, retrying\n",
					server_list[nservers], servers[nservers].addr);
			destroy_conn(&servers[nservers]);
		}
		log_info("loaded server [%s] %s\n",
				server_list[nservers], servers[nservers].addr);
	}
}

static void
sock_action(int signo, siginfo_t *info, void *context)
{
	for (int i = 0; i < nservers; ++i) {
		if (info->si_fd == *servers[i].fd) {
			int n; /* handle all new messages for matching server */
			char line_buf[BUFSIZE];
			while((n = SSL_read(servers[i].sock, line_buf, sizeof(line_buf)-1)) > 0) {
				line_buf[n] = '\0';
				log_recv("%s", line_buf);

				/* prepare args and let module handler thread take care of it */
				mod_arg *args = malloc(sizeof(mod_arg));
				clock_gettime(CLOCK_MONOTONIC_RAW, &args->ts);
				args->conn = &servers[i];
				strcpy(args->line, line_buf);
				pthread_create(&args->thr, NULL, (void *)handle_modules, args);
			}
		}
	}
}

static void
handle_abort()
{
	log_err("received SIGABRT, probably stuck, killing main thread\n");
	/* in a reality where I care I'd free all the resources here */
	FILE *crashf = fopen("/tmp/k88_crash", "w+");
	fputs("watchdog timeout", crashf);
	fclose(crashf);
	exit(1);
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
	signal(SIGABRT, (void *)handle_abort);

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
