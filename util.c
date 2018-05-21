#include "util.h"

void
handle_raw(int *sock, bool *reconnect, char *line)
{
    /* use total msg length for all temporary strings,
     * possible to do more efficient but good enough */
    int msglen = strlen(line);

    char *raw_msg = strchr(line, ' ') + 1;
    char *msgtype = strchr(raw_msg, ' ') + 1;

    if (!strncmp(msgtype, "PRIVMSG", 7)) {
        /* filter out info from raw PRIVMSG string */        
        char msg[msglen]; 
        char sender[msglen]; 
        char *channel = strchr(msgtype, ' ') + 1;
        strcpy(sender, raw_msg+1);
        strchr(sender, '!')[0] = '\0';
        strcpy(msg, strchr(channel, ':')+1);
        strchr(channel, ' ')[0] = '\0';

        /* pull chaninfo from db */
        char *chandb = db_getdb("db", channel);
        char prefix = db_getchr(db_entry(chandb, "prefix"));

        /* catch bits */
        char *bitstr = strchr(line, ';') + 1;
        if (!strncmp(bitstr, "bits=", 5)) {
            int bits = 0;
            sscanf(bitstr, "bits=%d", &bits);
            printf("[ (!) ] %s gave %d bits in %s\n", sender, bits, channel);
        }

        /* ctcp */
        if (!strncmp(msg, "VERSION", 9)) {
            puts("[ (!) ] ctcp version"); 
            send_raw(sock, 0, "NOTICE %s :VERSION socket.h\r\n", DEST);
        } else if (!strncmp(msg, "PING ", 6)) {
            puts("[ (!) ] ctcp ping"); 
            send_raw(sock, 0, "NOTICE %s :PING %u\r\n", DEST, 88 );
        } else if (!strncmp(msg, "TIME", 6)) {
            puts("[ (!) ] ctcp time"); 
            send_raw(sock, 0, "NOTICE %s :TIME %u\r\n", DEST, (unsigned)time(NULL));
        }

        char user[msglen];
        strcpy(user, raw_msg+1);
        strchr(user, '!')[0] = '\0';
        /* greeting */
        if (!db_exists(db_entry(chandb, "greeted", user))) {
            db_mkitem(db_entry(chandb, "greeted", user));
            send_raw(sock, 0, "PRIVMSG %s :%s VoHiYo\r\n", DEST, user);
        }

        /* afk checking */
        if (db_exists(db_entry(chandb, "afk", user))) {
            send_raw(sock, 0, "PRIVMSG %s :%s is no longer afk\r\n", DEST, user);
            db_del(db_entry(chandb, "afk", user));
        }

        /* commands */
        if (msg[0] == prefix) {
            printf("[ (!) ] '%c' prefixed command detected\n", prefix);
            strchr(msg, '\r')[0] = '\0';

            /* admin commands */
            if (!strncmp(raw_msg, ADMINSTR, strlen(ADMINSTR))) {
                if (!strncmp(msg+1, "join ", 5)) {
                    join_chan(sock, strchr(msg, ' ') + 1);
                } else if (!strncmp(msg+1, "part ", 5)) {
                    part_chan(sock, strchr(msg, ' ') + 1);
                } else if (!strncmp(msg+1, "title ", 6)) {
                    /* push to title string */
                    send_raw(sock, 0, "TOPIC %s\r\n", channel);
                } else if (!strncmp(msg+1, "prefix ", 7)) {
                    send_raw(sock, 0, "PRIVMSG %s :updating prefix in %s from '%c' to '%c'\r\n",
                            channel, channel, prefix, msg[8]);
                    db_setchr(db_entry(chandb, "prefix"), msg[8]);
                } else if (!strncmp(msg+1, "reconnect", 9)) {
                    puts("[ (!) ] reconnect notice received");
                    send_raw(sock, 0, "PRIVMSG %s :received a reconnect notice TehePelo\r\n", CHAN);
                    *reconnect = true;
                }
            } 

            /* public commands */
            if (!strncmp(msg+1, "checkme", 7)) {
                if (!strncmp(raw_msg, ADMINSTR, strlen(ADMINSTR))) {
                    send_raw(sock, 0, "PRIVMSG %s :user status: admin\r\n", DEST);
                } else {
                    send_raw(sock, 0, "PRIVMSG %s :user status: user\r\n", DEST);
                }
            } else if (!strncmp(msg+1, "vanish", 6)) {
                send_raw(sock, 0, "PRIVMSG %s :.timeout %s 1\r\n", DEST, user);
            } else if (!strncmp(msg+1, "t ", 2)) {
                char tag[msglen];
                sscanf(msg+3, "%s", tag);
                printf("tag: '%s'\n", tag);

                send_raw(sock, 0, "PRIVMSG %s :%s\r\n", DEST, 
                        db_exists(db_entry(chandb, "tags", tag))
                        ? db_getstr(db_entry(chandb, "tags", tag))
                        : "tag not found");
            } else if (!strncmp(msg+1, "tnew ", 5)) {
                char tag[msglen];
                char content[msglen];
                sscanf(msg+6, "%s", tag);
                strcpy(content, strchr(strchr(msg+1, ' ')+1, ' ') 
                        ? strchr(strchr(msg+1, ' ')+1, ' ')+1
                        : "");

                if (db_exists(db_entry(chandb, "tags", tag))) {
                    send_raw(sock, 0, "PRIVMSG %s :tag already exists\r\n", DEST);
                } else if (strlen(content) > 0) {
                    db_setstr(db_entry(chandb, "tags", tag), content);
                    send_raw(sock, 0, "PRIVMSG %s :tag created\r\n", DEST);
                } else {
                    send_raw(sock, 0, "PRIVMSG %s :no tag content given\r\n", DEST);
                }
            } else if (!strncmp(msg+1, "tedit ", 6)) {
                char tag[msglen];
                char content[msglen];
                sscanf(msg+7, "%s", tag);
                strcpy(content, strchr(strchr(msg+1, ' ')+1, ' ') 
                        ? strchr(strchr(msg+1, ' ')+1, ' ')+1
                        : "");

                if (!db_exists(db_entry(chandb, "tags", tag))) {
                    send_raw(sock, 0, "PRIVMSG %s :tag not found\r\n", DEST);
                } else if (strlen(content) > 0) {
                    db_setstr(db_entry(chandb, "tags", tag), content);
                    send_raw(sock, 0, "PRIVMSG %s :tag updated\r\n", DEST);
                } else {
                    send_raw(sock, 0, "PRIVMSG %s :no tag content given\r\n", DEST);
                }
            } else if (!strncmp(msg+1, "c ", 2)) {
                char counter[msglen];
                sscanf(msg+3, "%s", counter);
                printf("counter: '%s'\n", counter);

                if (db_exists(db_entry(chandb, "counters", counter))) {
                    int c = db_getnum(db_entry(chandb, "counters", counter))+1;
                    send_raw(sock, 0, "PRIVMSG %s :%s counter: %d\r\n", DEST,
                            counter, c);
                    db_setnum(db_entry(chandb, "counters", counter), c);
                } else {
                    send_raw(sock, 0, "PRIVMSG %s :counter not found\r\n", DEST);
                }
            } else if (!strncmp(msg+1, "cnew ", 5)) {
                char counter[msglen];
                sscanf(msg+6, "%s", counter);

                if (db_exists(db_entry(chandb, "counters", counter))) {
                    send_raw(sock, 0, "PRIVMSG %s :counter already exists\r\n", DEST);
                } else {
                    db_setnum(db_entry(chandb, "counters", counter), 0);
                    send_raw(sock, 0, "PRIVMSG %s :counter created\r\n", DEST);
                }
            } else if (!strncmp(msg+1, "cedit ", 6)) {
                char counter[msglen];
                int content;
                sscanf(msg+7, "%s %d", counter, &content);

                if (!db_exists(db_entry(chandb, "counters", counter))) {
                    send_raw(sock, 0, "PRIVMSG %s :counter not found\r\n", DEST);
                } else {
                    db_setnum(db_entry(chandb, "counters", counter), content);
                    send_raw(sock, 0, "PRIVMSG %s :counter updated\r\n", DEST);
                }
            } else if (!strncmp(msg+1, "afk", 3)) {
                char content[msglen];
                strcpy(content, strchr(msg+1, ' ') 
                        ? strchr(msg+1, ' ')+1
                        : "no context");
                send_raw(sock, 0, "PRIVMSG %s :%s is now afk: %s\r\n", DEST,
                            user, content);
                db_setstr(db_entry(chandb, "afk", user), content);
            } else if (!strncmp(msg+1, "whereis ", 8)) {
                char quser[msglen];
                sscanf(msg+9, "%s", quser);

                if (db_exists(db_entry(chandb, "afk", quser))) {
                    send_raw(sock, 0, "PRIVMSG %s :%s is afk: %s\r\n", DEST, quser,
                            db_getstr(db_entry(chandb, "afk", quser)));
                } else {
                    send_raw(sock, 0, "PRIVMSG %s :¯\\_(ツ)_/¯\r\n", DEST);
                }
            }

        } else if (!strncmp(msg, ".bots", 5)) {
            send_raw(sock, 0, "PRIVMSG %s :Reporting in! [C]\r\n", DEST);
        } 

        /* free db vars */
        free(chandb);
    } else if (!strncmp(msgtype, "USERNOTICE", 10)) {
        char channel[msglen];
        strcpy(channel, strchr(msgtype, ' ')+1);
        if (strchr(channel, ' ') != NULL) {
            puts("[ (!) ] not an empty sub message");
            strchr(channel, ' ')[0] = '\0';
        }

        char user[msglen]; 
        strcpy(user, strchr(strchr(strchr(line, ';')+1, ';')+1, '=')+1);
        strchr(user, ';')[0] = '\0';

        printf("[ (!) ] %s did a nice thing in %s\n", user, channel);
    } else if (strpos("RECONNECT", line) > -1) {
        puts("[ (!) ] reconnect notice received");
        send_raw(sock, 0, "PRIVMSG %s :received a reconnect notice TehePelo\r\n", CHAN);
        *reconnect = true;
    } 
}

void
send_raw(int *sock, bool silent, char *msgformat, ...)
{
    char buf[2000];
    va_list args;
    va_start(args, msgformat);
    vsnprintf(buf, 2000, msgformat, args);
    va_end(args);

    /* hackishly overwrite PRIVMSG with WHISPER for twitch pms */
    if (!strncmp(buf, "PRIVMSG", 7) && buf[8] != '#') {
        char whisper[] = "WHISPER";
        for (int i = 0; i < 7; ++i) {
            buf[i] = whisper[i];
        }
    }

    if (send(*sock, buf, strlen(buf), 0) == -1 && !silent) {
        fprintf(stderr, "[ !!! ] failed to send: '%s'", buf);
    } else if (!silent) {
        printf("[ >>> ] %s", buf);
    }
}

void
join_chan(int *sock, char *chan)
{
    /* check if db is alright */
    char *db = db_getdb("db", chan);
    if (!db_exists(db)) {
        db_init(db);
        db_init(db_entry(db, "greeted"));
        db_init(db_entry(db, "bux"));
        db_init(db_entry(db, "afk"));
        db_init(db_entry(db, "tags"));
        db_init(db_entry(db, "counters"));
        db_setstr(db_entry(db, "prefix"), ";");
    }
    free(db);

    db_mkitem(db_entry("db", "channels", chan));
    send_raw(sock, 0, "JOIN %s\r\n", chan);
}

void
part_chan(int *sock, char *chan)
{
    db_del(db_entry("db", "channels", chan));
    send_raw(sock, 0, "PART %s\r\n", chan);
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
