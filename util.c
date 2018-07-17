#include "util.h"
pthread_t _t;

/* djb2 string hash function */
unsigned long
hash(char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void
start_handle_bux()
{
    int rc;
    if ((rc = pthread_create(&_t, NULL, (void *) handle_bux, NULL))) {
        printf("[ !!! ] failed to create handle_bux thread: %d\n", rc);
    } else {
        pthread_detach(_t);
    }
}

void
handle_bux()
{
    puts("[ (!) ] awarding bux...");
    llist *chans = malloc(sizeof(llist));
    db_list(db_entry("db", "channels"), chans);


    /* loop over every channel */
    while(chans->head != NULL && 
            strlen(chans->head->name) > 1 &&
            chans->head->name[0] == '#') {

        /* loop over every active user in channel */
        while (db_file(db_entry("db", chans->head->name, "active"))) {

            /* add bux to active user */
            int old_bux = db_exists(db_entry("db", chans->head->name, "bux", 
                        db_file(db_entry("db", chans->head->name, "active")))) ?
                        db_getnum(db_entry("db", chans->head->name, "bux", 
                        db_file(db_entry("db", chans->head->name, "active")))) : 0;

            db_setnum(db_entry("db", chans->head->name, "bux", 
                        db_file(db_entry("db", chans->head->name, "active"))),
                        old_bux + BUX_AMOUNT);

            /* move on to next user */
            db_del(db_entry("db", chans->head->name, "active",
                        db_file(db_entry("db", chans->head->name, "active"))));
        }

        /* move on to next channel */
        pop(chans, chans->head->name);
    }

    puts("[ (!) ] done awarding bux");
    free(chans);
    alarm(BUX_INTERVAL);
    pthread_exit(NULL);
}

bool
check_db(char *chandb)
{
    int fixed = 0;
    char *dbs[] = {
        db_getdb(chandb, "greeted"), 
        db_getdb(chandb, "bux"), 
        db_getdb(chandb, "afk"), 
        db_getdb(chandb, "tags"), 
        db_getdb(chandb, "counters"), 
        db_getdb(chandb, "settings"),
        db_getdb(chandb, "active"),
        db_getdb("db", "mail"),
        db_getdb("db", "channels")
    };

    for (int i = 0; i < 9; ++i) {
        if (!db_exists(dbs[i])) {
            printf("[ (!) ] added db: %s\n", dbs[i]);
            db_init(dbs[i]);
            fixed++;
        }
        free(dbs[i]);
    }
    if (!db_exists(db_entry(chandb, "prefix"))) {
        db_setchr(db_entry(chandb, "prefix"), ';');
        fixed++;
    }
    if (!db_exists(db_entry(chandb, "greeting_format"))) {
        db_setstr(db_entry(chandb, "greeting_format"), "%s VoHiYo");
        fixed++;
    }
    return (fixed > 0);
}

int
strpos(char *match, char *str)
{
    if (strlen(match) > strlen(str)) {
        return -1;
    }

    int match_len = strlen(match);
    int str_len = strlen(str);

    for (int i = 0; i < str_len - match_len+1; ++i) {
        if (match[0] == str[i]) {
            int ii = 0;
            while (match[ii] == str[i + ii]) {
                if (ii+1 == match_len) {
                    return i;
                }
                ++ii;
            }
        }
    }
    return -1;
}
