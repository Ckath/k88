#ifndef MODULES_H
#define MODULES_H
#include "irc.h"
#include <stdbool.h>

typedef struct {
	char name[256];
	void (*rawmsg)(irc_conn *, char *, char *);
	void (*privmsg)(irc_conn *, char *, char *, char *, char *);
	void (*cmdmsg)(irc_conn *, char *, char *, char *, char *, bool);
	bool default_enable;
} module;

void mods_new(char *name, bool default_enable);
void mods_rawmsg_handler(void *handler);
void mods_privmsg_handler(void *handler);
void mods_cmdmsg_handler(void *handler);
char *mods_get_config(char *index, char *item);
char *mods_get_prefix(irc_conn *conn, char *index);
int mods_set_config(char *index, char *item, char *value);
char **mods_list();
void init_modules();
void handle_modules(irc_conn *server, char *line);

/* terrible macro bodging to hide how bad the module system is */
#define NEWMOD all_mods[all_mods_len-1]

#endif
