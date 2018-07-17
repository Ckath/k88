#include "commands.h"

command *commands[24];

void
init_commands()
{
    /* checkme command */
    commands[0] = malloc(sizeof(command));
    strcpy(commands[0]->trigger, "checkme");
    commands[0]->perm = USER;
    commands[0]->func = (void *) checkme;

    /* vanish command */
    commands[1] = malloc(sizeof(command));
    strcpy(commands[1]->trigger, "vanish");
    commands[1]->perm = USER;
    commands[1]->func = (void *) vanish;

    /* 8ball command */
    commands[2] = malloc(sizeof(command));
    strcpy(commands[2]->trigger, "8ball ");
    commands[2]->perm = USER;
    commands[2]->func = (void *) ball;

    /* tag command */
    commands[3] = malloc(sizeof(command));
    strcpy(commands[3]->trigger, "t ");
    commands[3]->perm = USER;
    commands[3]->func = (void *) t;

    /* new tag command */
    commands[4] = malloc(sizeof(command));
    strcpy(commands[4]->trigger, "tnew ");
    commands[4]->perm = MOD;
    commands[4]->func = (void *) tnew;

    /* edit tag command */
    commands[5] = malloc(sizeof(command));
    strcpy(commands[5]->trigger, "tedit ");
    commands[5]->perm = MOD;
    commands[5]->func = (void *) tedit;

    /* counter command */
    commands[6] = malloc(sizeof(command));
    strcpy(commands[6]->trigger, "c ");
    commands[6]->perm = USER;
    commands[6]->func = (void *) c;

    /* new counter command */
    commands[7] = malloc(sizeof(command));
    strcpy(commands[7]->trigger, "cnew ");
    commands[7]->perm = MOD;
    commands[7]->func = (void *) cnew;

    /* edit counter command */
    commands[8] = malloc(sizeof(command));
    strcpy(commands[8]->trigger, "cedit ");
    commands[8]->perm = MOD;
    commands[8]->func = (void *) cedit;

    /* afk command */
    commands[9] = malloc(sizeof(command));
    strcpy(commands[9]->trigger, "afk");
    commands[9]->perm = USER;
    commands[9]->func = (void *) afk;

    /* whereis command */
    commands[10] = malloc(sizeof(command));
    strcpy(commands[10]->trigger, "whereis ");
    commands[10]->perm = USER;
    commands[10]->func = (void *) whereis;

    /* mail command */
    commands[11] = malloc(sizeof(command));
    strcpy(commands[11]->trigger, "mail ");
    commands[11]->perm = USER;
    commands[11]->func = (void *) mail;

    /* bux command */
    commands[12] = malloc(sizeof(command));
    strcpy(commands[12]->trigger, "bux");
    commands[12]->perm = USER;
    commands[12]->func = (void *) bux;

    /* give bux command */
    commands[13] = malloc(sizeof(command));
    strcpy(commands[13]->trigger, "give ");
    commands[13]->perm = USER;
    commands[13]->func = (void *) give;

    /* gamble bux command */
    commands[14] = malloc(sizeof(command));
    strcpy(commands[14]->trigger, "gamble ");
    commands[14]->perm = USER;
    commands[14]->func = (void *) gamble;

    /* timeout command */
    commands[15] = malloc(sizeof(command));
    strcpy(commands[15]->trigger, "$timeout ");
    commands[15]->perm = USER;
    commands[15]->func = (void *) timeout;

    /* join channel command */
    commands[16] = malloc(sizeof(command));
    strcpy(commands[16]->trigger, "join ");
    commands[16]->perm = BOT_OWNER;
    commands[16]->func = (void *) join;

    /* part channel command */
    commands[17] = malloc(sizeof(command));
    strcpy(commands[17]->trigger, "part ");
    commands[17]->perm = BOT_OWNER;
    commands[17]->func = (void *) part;

    /* prefix command */
    commands[18] = malloc(sizeof(command));
    strcpy(commands[18]->trigger, "prefix ");
    commands[18]->perm = BOT_MOD;
    commands[18]->func = (void *) prefix;

    /* enable setting command */
    commands[19] = malloc(sizeof(command));
    strcpy(commands[19]->trigger, "enable ");
    commands[19]->perm = BOT_MOD;
    commands[19]->func = (void *) enable;

    /* disable setting command */
    commands[20] = malloc(sizeof(command));
    strcpy(commands[20]->trigger, "disable ");
    commands[20]->perm = BOT_MOD;
    commands[20]->func = (void *) disable;

    /* checkdb command */
    commands[21] = malloc(sizeof(command));
    strcpy(commands[21]->trigger, "checkdb");
    commands[21]->perm = BOT_MOD;
    commands[21]->func = (void *) checkdb;

    /* regreet command */
    commands[22] = malloc(sizeof(command));
    strcpy(commands[22]->trigger, "regreet");
    commands[22]->perm = BOT_MOD;
    commands[22]->func = (void *) regreet;

    /* setgreeting command */
    commands[23] = malloc(sizeof(command));
    strcpy(commands[23]->trigger, "setgreeting ");
    commands[23]->perm = BOT_MOD;
    commands[23]->func = (void *) setgreeting;
}

void 
checkme(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    if (!strncmp(user, ADMINSTR, strlen(ADMINSTR))) {
        send_raw(conn, 0, "PRIVMSG %s :user status: admin\r\n", DEST);
    } else {
        send_raw(conn, 0, "PRIVMSG %s :user status: user\r\n", DEST);
    }
}

void 
vanish(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    if (db_exists(db_entry(chandb, "settings", "novanish"))) {
        return;
    }

    send_raw(conn, 0, "PRIVMSG %s :.timeout %s 1\r\n", DEST, user);
}

void 
ball(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char *ball[] = {
        "it is certain",
        "it is decidedly so",
        "without a doubt",
        "yes definitely",
        "you may rely on it",
        "as I see it, yes",
        "most likely",
        "outlook good",
        "yes",
        "signs point to yes",
        "reply hazy try again",
        "ask again later",
        "idk kev 4Head",
        "better not tell you now",
        "cannot predict now",
        "concentrate and ask again later",
        "don't count on it",
        "my reply is no",
        "my sources say no",
        "outlook not so good",
        "my sources say no",
        "very doubtful",
        "maybe Kappa",
        "like I care LUL",
        "asking a scuffed irc bot for advice LUL"
    };

    srand(time(NULL));
    send_raw(conn, 0, "PRIVMSG %s :%s\r\n", DEST, ball[rand()%25]);
}

void 
t(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char tag[BUF_SIZE];
    sscanf(msg+3, "%s", tag);
    printf("tag: '%s'\n", tag);

    send_raw(conn, 0, "PRIVMSG %s :%s\r\n", DEST, 
            db_exists(db_entry(chandb, "tags", tag))
            ? db_getstr(db_entry(chandb, "tags", tag))
            : "tag not found");
}

void 
tnew(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char tag[BUF_SIZE];
    char content[BUF_SIZE];
    sscanf(msg+6, "%s", tag);
    strcpy(content, strchr(strchr(msg+1, ' ')+1, ' ') 
            ? strchr(strchr(msg+1, ' ')+1, ' ')+1
            : "");

    if (db_exists(db_entry(chandb, "tags", tag))) {
        send_raw(conn, 0, "PRIVMSG %s :tag already exists\r\n", DEST);
    } else if (strlen(content) > 0) {
        db_setstr(db_entry(chandb, "tags", tag), content);
        send_raw(conn, 0, "PRIVMSG %s :tag created\r\n", DEST);
    } else {
        send_raw(conn, 0, "PRIVMSG %s :no tag content given\r\n", DEST);
    }
}

void 
tedit(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char tag[BUF_SIZE];
    char content[BUF_SIZE];
    sscanf(msg+7, "%s", tag);
    strcpy(content, strchr(strchr(msg+1, ' ')+1, ' ') 
            ? strchr(strchr(msg+1, ' ')+1, ' ')+1
            : "");

    if (!db_exists(db_entry(chandb, "tags", tag))) {
        send_raw(conn, 0, "PRIVMSG %s :tag not found\r\n", DEST);
    } else if (strlen(content) > 0) {
        db_setstr(db_entry(chandb, "tags", tag), content);
        send_raw(conn, 0, "PRIVMSG %s :tag updated\r\n", DEST);
    } else {
        send_raw(conn, 0, "PRIVMSG %s :no tag content given\r\n", DEST);
    }
}

void 
c(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char counter[BUF_SIZE];
    sscanf(msg+3, "%s", counter);
    printf("counter: '%s'\n", counter);

    if (db_exists(db_entry(chandb, "counters", counter))) {
        int c = db_getnum(db_entry(chandb, "counters", counter))+1;
        send_raw(conn, 0, "PRIVMSG %s :%s counter: %d\r\n", DEST,
                counter, c);
        db_setnum(db_entry(chandb, "counters", counter), c);
    } else {
        send_raw(conn, 0, "PRIVMSG %s :counter not found\r\n", DEST);
    }
}

void 
cnew(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char counter[BUF_SIZE];
    sscanf(msg+6, "%s", counter);

    if (db_exists(db_entry(chandb, "counters", counter))) {
        send_raw(conn, 0, "PRIVMSG %s :counter already exists\r\n", DEST);
    } else {
        db_setnum(db_entry(chandb, "counters", counter), 0);
        send_raw(conn, 0, "PRIVMSG %s :counter created\r\n", DEST);
    }
}

void 
cedit(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char counter[BUF_SIZE];
    int content;
    sscanf(msg+7, "%s %d", counter, &content);

    if (!db_exists(db_entry(chandb, "counters", counter))) {
        send_raw(conn, 0, "PRIVMSG %s :counter not found\r\n", DEST);
    } else {
        db_setnum(db_entry(chandb, "counters", counter), content);
        send_raw(conn, 0, "PRIVMSG %s :counter updated\r\n", DEST);
    }
}

void 
afk(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char content[BUF_SIZE];
    strcpy(content, strchr(msg+1, ' ') 
            ? strchr(msg+1, ' ')+1
            : "no context");

    db_setstr(db_entry(chandb, "afk", user), content);
    send_raw(conn, 0, "PRIVMSG %s :%s is now afk: %s\r\n", DEST,
                user, content);
}

void 
whereis(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char quser[BUF_SIZE];
    sscanf(msg+9, "%s", quser);

    /* make sure nick is lowercase before looking it up */
    TOLOWER(quser);

    if (db_exists(db_entry(chandb, "afk", quser))) {
        send_raw(conn, 0, "PRIVMSG %s :%s is afk: %s\r\n", DEST, quser,
                db_getstr(db_entry(chandb, "afk", quser)));
    } else {
        send_raw(conn, 0, "PRIVMSG %s :¯\\_(ツ)_/¯\r\n", DEST);
    }
}

void 
mail(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char *muser = malloc(sizeof(char)*BUF_SIZE);
    char *pmuser = muser;
    char content[BUF_SIZE];
    sscanf(msg+6, "%s", muser);
    strcpy(content, strchr(strchr(msg+1, ' ')+1, ' ') 
            ? strchr(strchr(msg+1, ' ')+1, ' ')+1
            : "");

    /* make sure nick is lowercase before storing it */
    TOLOWER(muser);
    if (muser[0] == '@') {
        muser++;
    }

    db_init(db_entry("db", "mail", muser));
    db_setstr(db_entry("db", "mail", muser, user), content);
    send_raw(conn, 0, "PRIVMSG %s :will pass that on to %s when I spot them\r\n", DEST, 
            muser);
    free(pmuser);
}

void 
bux(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char *buser = malloc(BUF_SIZE * sizeof(char));
    char *pbuser = buser;
    sscanf(msg+5, "%s", buser);

    if (msg[4] == ' ') {
        if (buser[0] == '@') {
            buser = buser+1;
        }

        /* make sure nick is lowercase before checking */
        TOLOWER(buser);

        if (db_exists(db_entry("db", channel, "bux", buser))) {
            send_raw(conn, 0, "PRIVMSG %s :%s's %sbux: %d\r\n", DEST, 
                    buser, channel+1, 
                    db_getnum(db_entry(chandb, "bux", buser)));
        }
    } else {
        if (db_exists(db_entry("db", channel, "bux", user))) {
            send_raw(conn, 0, "PRIVMSG %s :%s's %sbux: %d\r\n", DEST, 
                    user, channel+1, 
                    db_getnum(db_entry(chandb, "bux", user)));
        }
    }
    free(pbuser);
}

void 
give(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    char *guser = malloc(BUF_SIZE * sizeof(char));
    char *pguser = guser;
    int user_bux;
    int give_amount;
    sscanf(msg+6, "%s %d", guser, &give_amount);

    /* make sure nick is lowercase before storing it */
    TOLOWER(guser);
    if (guser[0] == '@') {
        guser++;
    }

    give_amount = abs(give_amount);
    
    if (!db_exists(db_entry(chandb, "bux", guser))) {
        send_raw(conn, 0, "PRIVMSG %s :%s hasn't been active enough to deserve bux\r\n", 
                DEST, guser);
    } else if (!db_exists(db_entry(chandb, "bux", user)) || 
            (user_bux = db_getnum(db_entry(chandb, "bux", user))) < give_amount) {
        send_raw(conn, 0, "PRIVMSG %s :insufficient bux\r\n", DEST);
    } else {
        db_setnum(db_entry(db_entry(chandb, "bux", guser)), 
                db_getnum(db_entry(chandb, "bux", guser)) + give_amount);
        db_setnum(db_entry(db_entry(chandb, "bux", user)), 
                db_getnum(db_entry(chandb, "bux", user)) - give_amount);
        send_raw(conn, 0, "PRIVMSG %s :%s gave %s %d %sbux\r\n", DEST,
                user, guser, give_amount, channel+1);
    }

    free(pguser);
}

void 
gamble(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    int user_bux = 0;
    int gamble_amount = 0;
    char gamble_all[4];
    sscanf(msg+8, "%d", &gamble_amount);
    sscanf(msg+8, "%3s", gamble_all);
    
    srand(time(NULL));
    gamble_amount = abs(gamble_amount);

    if ((!db_exists(db_entry(chandb, "bux", user)) || 
            (user_bux = db_getnum(db_entry(chandb, "bux", user))) < gamble_amount) 
            && strcmp("all", gamble_all)) {
        send_raw(conn, 0, "PRIVMSG %s :insufficient bux\r\n", DEST);
    } else if (rand()%2) {
        gamble_amount = !strcmp("all", gamble_all) ? user_bux : gamble_amount;
        user_bux -= gamble_amount;

        db_setnum(db_entry(chandb, "bux", user), 
                user_bux + (int) (gamble_amount * GAMBLE_PROFIT));
        send_raw(conn, 0, "PRIVMSG %s :%s just won %d %sbux, now has %d %sbux\r\n", DEST,
                user, (int)(gamble_amount*GAMBLE_PROFIT-gamble_amount), channel+1,
                db_getnum(db_entry(chandb, "bux", user)), channel+1);
    } else {
        gamble_amount = !strcmp("all", gamble_all) ? user_bux : gamble_amount;
        db_setnum(db_entry(chandb, "bux", user), 
                user_bux - gamble_amount);
        send_raw(conn, 0, "PRIVMSG %s :%s just lost %d %sbux, now has %d %sbux\r\n", 
                DEST, user, gamble_amount, channel+1,
                db_getnum(db_entry(chandb, "bux", user)), channel+1);
    }
}

void 
timeout(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    int user_bux = 0;
    char tuser[BUF_SIZE];
    sscanf(msg+10, "%s", tuser);

    if (db_exists(db_entry(chandb, "bux", user)) &&
            (user_bux = db_getnum(db_entry(chandb, "bux", user))) 
            > TIMEOUT_COST+1) {
        db_setnum(db_entry(chandb, "bux", user), user_bux-TIMEOUT_COST);
        send_raw(conn, 0, "PRIVMSG %s :%s wasted %d %sbux to timeout %s LUL\r\n", DEST,
                user, TIMEOUT_COST, channel+1, tuser);
        send_raw(conn, 0, "PRIVMSG %s :.timeout %s %d\r\n", DEST,
                tuser, TIMEOUT_DURATION);
    }
}

void
join(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    join_chan(conn, strchr(msg, ' ') + 1);
}

void
part(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    part_chan(conn, strchr(msg, ' ') + 1);
}

void
prefix(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    send_raw(conn, 0, "PRIVMSG %s :updating prefix in %s from '%c' to '%c'\r\n",
            channel, channel, prefix, msg[8]);
    db_setchr(db_entry(chandb, "prefix"), msg[8]);
}

void
enable(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    db_mkitem(db_entry(chandb, "settings", strchr(msg, ' ') + 1));
    send_raw(conn, 0, "PRIVMSG %s :enabled %s\r\n", DEST, strchr(msg, ' ') + 1);
}

void
disable(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    if (db_exists(db_entry(chandb, "settings", strchr(msg, ' ') + 1))) {
        db_del(db_entry(chandb, "settings", strchr(msg, ' ') + 1));
        send_raw(conn, 0, "PRIVMSG %s :disabled %s\r\n", DEST, strchr(msg, ' ') + 1);
    } else {
        send_raw(conn, 0, "PRIVMSG %s :setting not enabled\r\n", DEST);
    }
}

void
checkdb(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    send_raw(conn, 0, "PRIVMSG %s :%s\r\n", DEST, 
            check_db(chandb) ? "db fixed" : "db is fine");
}

void
regreet(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    while (db_file(db_entry(chandb, "greeted"))) {
        db_del(db_entry(chandb, "greeted", db_file(db_entry(chandb, "greeted"))));
    }
    send_raw(conn, 0, "PRIVMSG %s :greeted cache cleared, regreeting everyone\r\n", 
            DEST);
}

void
setgreeting(irc_conn *conn, char *msg, char *user, char *channel, char *chandb)
{
    db_setstr(db_entry(chandb, "greeting_format"), msg+13);
    send_raw(conn, 0, "PRIVMSG %s :set greeting in %s to: '%s'\r\n", DEST,
    channel, db_getstr(db_entry(chandb, "greeting_format")));
}
