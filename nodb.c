#include "nodb.h"

void 
db_init(char *path)
{
    mkdir(path, 0755);
}

int
db_mkitem(char *path)
{
    FILE *f = fopen(path, "w");
    fclose(f);
}

void
db_setnum(char *path, int num)
{
    FILE *f = fopen(path, "w");

    fprintf(f, "%d", num);
    fclose(f);
}

void
db_setstr(char *path, char *str)
{
    FILE *f = fopen(path, "w");

    fputs(str, f);
    fclose(f);
}

void
db_setchr(char *path, char chr)
{
    FILE *f = fopen(path, "w");

    fputc(chr, f);
    fclose(f);
}

int
db_getnum(char *path)
{
    int ret;
    FILE *f = fopen(path, "r");

    fscanf(f, "%d", &ret);
    fclose(f);
    return ret;
}

char *
db_getstr(char *path)
{
    static char ret[512];
    FILE *f = fopen(path, "r");

    fgets(ret, 512, f);
    fclose(f);
    return ret;
}

char
db_getchr(char *path)
{
    char ret;
    FILE *f = fopen(path, "r");

    fscanf(f, "%c", &ret);
    fclose(f);
    return ret;
}

bool
db_exists(char *path)
{
    struct stat buf;
    bool exists = (stat(path, &buf) == 0);

    return exists;
}

char *
(db_entry)(char *db, ...)
{
    static char db_entry[512];
    char *buf = malloc(512);

    strcpy(db_entry, db);

    va_list ap;
    va_start(ap, db);
    while (strcpy(buf, va_arg(ap, char *)) && strcmp(buf, "")) {
        strcat(db_entry, "/");
        strcat(db_entry, buf);
    }
    va_end(ap);

    free(buf);
    return db_entry;
}

char *
(db_getdb)(char *db, ...)
{
    char *db_ = malloc(512);
    char *buf = malloc(512);

    strcpy(db_, db);

    va_list ap;
    va_start(ap, db);
    while (strcpy(buf, va_arg(ap, char *)) && strcmp(buf, "")) {
        strcat(db_, "/");
        strcat(db_, buf);
    }
    va_end(ap);

    free(buf);
    return db_;
}

void
db_del(char *path)
{
    remove(path);
}

void
db_list(char *path, llist *entries)
{
    DIR *d;
    struct dirent *dir;

    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type != DT_DIR) {
                push(entries, dir->d_name);
            }
        }
        closedir(d);
    }
}

void
db_listdbs(char *path, llist *dbs)
{
    DIR *d;
    struct dirent *dir;

    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] == '#') {
                push(dbs, dir->d_name);
            }
        }
        closedir(d);
    }
}

char *
db_file(char *path)
{
    DIR *d;
    struct dirent *dir;

    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type != DT_DIR) {
                closedir(d);
                return dir->d_name;
            }
        }
    }
    return "thisdirectoryisdefinitelyemptyforsure";
}
