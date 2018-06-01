#ifndef NODB_H
#define NODB_H

#include "intern.h"
#include "extern.h"

void db_init(char *path);
int db_mkitem(char *path);
void db_setnum(char *path, int num);
void db_setstr(char *path, char *str);
void db_setchr(char *path, char chr);
int db_getnum(char *path);
char *db_getstr(char *path);
char db_getchr(char *path);
bool db_exists(char *path);
char *db_entry(char *db, ...);
char *db_getdb(char *db, ...);
void db_del(char *path);
void db_list(char *path, llist *entries);
char *db_file(char *path);

/* questionable macros to avoid segfault on out of bounds va_arg call */
#define db_entry(...) db_entry(__VA_ARGS__, "")
#define db_getdb(...) db_getdb(__VA_ARGS__, "")

#endif
