#ifndef MODULES_H
#define MODULES_H
#include "irc.h"
#include <stdbool.h>
#include <pthread.h>

typedef struct {
	irc_conn *conn;
	char *index;
	char *chan;
	char *user;
	bool mod;
} msg_info;

typedef struct {
	char name[256];
	void (*rawmsg)(msg_info *, char *);
	void (*privmsg)(msg_info *, char *);
	void (*cmdmsg)(msg_info *, char *);
	void (*timed)(irc_conn *, char *, time_t);
	bool default_enable;
} module;

typedef struct {
	module *mods;
	size_t n;
} modlist;

typedef struct {
	irc_conn *conn;
	char line[BUFSIZE];
	pthread_t thr;
} mod_arg;

typedef struct {
	irc_conn *conn;
	int n;
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
