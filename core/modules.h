#ifndef MODULES_H
#define MODULES_H
#include "irc.h"
#include <stdbool.h>
#include <pthread.h>

typedef struct msg_info {
	irc_conn *conn;
	char *index;
	char *chan;
	char *user;
	char *userid;
	bool mod;
	bool cmd;
	struct timespec ts;
} msg_info;

typedef struct module {
	char name[256];
	void (*rawmsg)(msg_info *, char *);
	void (*privmsg)(msg_info *, char *);
	void (*cmdmsg)(msg_info *, char *);
	void (*timed)(irc_conn *, char *, time_t);
	bool default_enable;
} module;

typedef struct modlist {
	module *mods;
	size_t n;
} modlist;

typedef struct mod_arg {
	irc_conn *conn;
	char line[BUFSIZE];
	pthread_t thr;
	struct timespec ts;
} mod_arg;

typedef struct timed_arg {
	irc_conn *conn;
	int n;
	time_t t;
	pthread_t thr;
} timed_arg;

void mods_new(char *name, bool default_enable);
void mods_rawmsg_handler(void *handler);
void mods_privmsg_handler(void *handler);
void mods_cmdmsg_handler(void *handler);
void mods_timed_handler(void *handler);
char *mods_get_config(char *index, char *item);
char *mods_get_prefix(irc_conn *conn, char *index);
int mods_set_config(char *index, char *item, char *value);
char **mods_list();
void init_modules();
void timed_modules(timed_arg *args);
void handle_modules(mod_arg *args);

/* terrible macro bodging to hide how bad the module system is */
#define NEWMOD all.mods[all.n-1]

#endif
