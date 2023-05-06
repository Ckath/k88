#define _GNU_SOURCE
#include <fcntl.h>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <signal.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "socks.h"
#include "irc.h"
#include "../utils/strutils.h"

char init_ssl = 0;

int
init_conn(irc_conn *conn)
{
	/* SSL inits */
	if (!init_ssl) {
		wolfSSL_Init();
		init_ssl = 1;
	}

	/* create socket for connection */
	conn->fd = malloc(sizeof(int));
	while(init_sock(conn->fd, conn->addr, conn->port)) {
		log_info("reconnecting...\n");
		sleep(1);
	}
	conn->init = 0; /* ensure we init it again */
	conn->heartbeat = 0;
	conn->reconns++;

	/* setup SSL */
	conn->ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
	if (!conn->ctx) {
		log_err("wolfSSL_CTX_new error\n");
		return 1;
	}

	if ((conn->sock = wolfSSL_new(conn->ctx)) == NULL) {
		log_err("wolfSSL_new error\n");
	}

	if (wolfSSL_CTX_load_system_CA_certs(conn->ctx) != SSL_SUCCESS) {
		log_err("error loading system certs\n");
	}

	wolfSSL_set_fd(conn->sock, *conn->fd);
	wolfSSL_connect(conn->sock);

	/* everything done, configure fd options */
	fcntl(*conn->fd, F_SETFL,
			fcntl(*conn->fd, F_GETFL, 0)|O_ASYNC|O_NONBLOCK);
	fcntl(*conn->fd, F_SETOWN, getpid());
	fcntl(*conn->fd, F_SETSIG, SIGPOLL);

	log_info("attempting to identify...\n");
	if (conn->pass) {
		send_raw(conn, 0, "USER %s 0 * :%s\r\n", conn->nick, conn->nick);
		send_raw(conn, 0, "NICK %s\r\n", conn->nick);
		send_raw(conn, 1, "PASS %s\r\n", conn->pass);
	} else { /* znc, which requires pass, doesnt enjoy k88:k88 as user */
		send_raw(conn, 0, "USER k88 0 * :k88\r\n");
		send_raw(conn, 0, "NICK %s\r\n", conn->nick);
	}
	return 0;
}

void
reconnect_conn(irc_conn *conn)
{
	destroy_conn(conn);
	init_conn(conn);
	while (init_conn(conn)) {
		log_err("failed to reconnect server [%s] %s, retrying\n",
				conn->index, conn->addr);
		destroy_conn(conn);
	}
	log_info("reconnected server [%s] %s\n",
			conn->index, conn->addr);
}

void
destroy_conn(irc_conn *conn)
{
	if (conn->sock) {
		wolfSSL_free(conn->sock);
	}
	if (conn->ctx) {
		wolfSSL_CTX_free(conn->ctx);
	}
	if (conn->fd) {
		free(conn->fd);
	}
}

void
join_chans(irc_conn *conn, char *chans)
{
	char *chanlist = strdup(chans);
	char *chan = strtok(chanlist, ", ");
	while (chan) {
		send_raw(conn, 0, "JOIN %s\r\n", chan);
		chan = strtok(NULL, ", ");
	}
	free(chanlist);
	free(chan);
}

void
part_chans(irc_conn *conn, char *chans)
{
	char *chanlist = strdup(chans);
	char *chan = strtok(chanlist, ", ");
	while (chan) {
		send_raw(conn, 0, "PART %s\r\n", chan);
		chan = strtok(NULL, ", ");
	}
	free(chanlist);
	free(chan);
}

void
send_raw(irc_conn *conn, char silent, char *msgformat, ...)
{
	char buf[BUFSIZE];
	va_list args;
	va_start(args, msgformat);
	vsnprintf(buf, BUFSIZE, msgformat, args);
	va_end(args);

	if (conn->twitch) {
		strrplc(buf, "3", "");
		strrplc(buf, "4", "");
		strrplc(buf, "6", "");
		strrplc(buf, "7", "");
		strrplc(buf, "9", "");
		strrplc(buf, "10", "");
		strrplc(buf, "11", "");
		strrplc(buf, "12", "");
		strrplc(buf, "13", "");
		strrplc(buf, "", "");
		strrplc(buf, "", "");
	}

	if (wolfSSL_write(conn->sock, buf, strlen(buf)) < 0 && !silent) {
		log_err("failed to send: '%s'", buf);
		FILE *crashf = fopen("/tmp/k88_crash", "w+");
		fputs("error on SSL fd, probably crashed", crashf);
		fclose(crashf);
		reconnect_conn(conn);
	} else if (!silent) {
		log_send("%s", buf);
	}
}
