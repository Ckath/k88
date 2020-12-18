#define _GNU_SOURCE
#include <fcntl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <signal.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "socks.h"
#include "irc.h"

char init_ssl = 0;

int
init_conn(irc_conn *conn)
{
	/* SSL inits */
	if (!init_ssl) {
		SSL_library_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();
		init_ssl = 1;
	}

	/* create socket for connection */
	conn->fd = malloc(sizeof(int));
	while(init_sock(conn->fd, conn->addr, conn->port)) {
		puts("[ (!) ] reconnecting...");
		sleep(1);
	}
	conn->init = 0; /* ensure we init it again */
	conn->reconns++;

	/* setup SSL */
	conn->ctx = SSL_CTX_new(TLS_client_method());
	if (!conn->ctx) {
		ERR_print_errors_fp(stderr);
	}
	SSL_CTX_set_mode(conn->ctx, SSL_MODE_AUTO_RETRY|SSL_MODE_ASYNC);
	conn->sock = SSL_new(conn->ctx);
	SSL_set_fd(conn->sock, *conn->fd);
	SSL_set_connect_state(conn->sock);
	SSL_connect(conn->sock);
	printf("[ (!) ] connected with %s cipher\n", SSL_get_cipher(conn->sock));
	/* somethings not right here, bail (for retry) */
	if (!strcmp(SSL_get_cipher(conn->sock), "(NONE)")) {
		return 1;
	}

	/* everything done, configure fd options */
	fcntl(*conn->fd, F_SETFL,
			fcntl(*conn->fd, F_GETFL, 0)|O_ASYNC|O_NONBLOCK);
	fcntl(*conn->fd, F_SETOWN, getpid());
	fcntl(*conn->fd, F_SETSIG, SIGPOLL);

	puts("[ (!) ] attempting to identify...");
	send_raw(conn, 0, "USER %s 0 * :%s\r\n", conn->nick, "k88");
	send_raw(conn, 0, "NICK %s\r\n", conn->nick);
	if (conn->pass) {
		send_raw(conn, 1, "PASS %s\r\n", conn->pass);
	}
	return 0;
}

void
reconnect_conn(irc_conn *conn)
{
	destroy_conn(conn);
	init_conn(conn);
	while (init_conn(conn)) {
		fprintf(stderr, "[ !!! ] failed to reconnect server [%s] %s, retrying\n",
				conn->index, conn->addr);
		destroy_conn(conn);
	}
	printf("[ (!) ] reconnected server [%s] %s\n",
			conn->index, conn->addr);
}

void
destroy_conn(irc_conn *conn)
{
	if (conn->sock) {
		SSL_free(conn->sock);
	}
	if (conn->ctx) {
		SSL_CTX_free(conn->ctx);
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

	if (SSL_write(conn->sock, buf, strlen(buf)) < 0 && !silent) {
		ERR_print_errors_fp(stderr);
		fprintf(stderr, "[ !!! ] failed to send: '%s'", buf);
	} else if (!silent) {
		printf("[ >>> ] %s", buf);
	}
}
