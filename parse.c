#include "parse.h"

void
handle_raw(irc_conn *conn, char *line)
{
    /* use total msg length for all temporary strings,
     * possible to do more efficient but good enough */
    int msglen = strlen(line);

    char *raw_msg = line;
    char *msgtype = strchr(raw_msg, ' ') + 1;

    if (!strncmp(msgtype, "PRIVMSG", 7)) {
        /* filter out info from raw PRIVMSG string */        
        char msg[msglen]; 
        char user[msglen]; 
        char *channel = strchr(msgtype, ' ') + 1;
        strcpy(user, raw_msg+1);
        strchr(user, '!')[0] = '\0';
        strcpy(msg, strchr(channel, ':')+1);
        strchr(channel, ' ')[0] = '\0';

        /* pull chaninfo from db */
        char *chandb = db_getdb("db", channel);
        char prefix = db_getchr(db_entry(chandb, "prefix"));

        /* handle activity */
        db_mkitem(db_entry(chandb, "active", user));

        /* catch bits */
        /* TODO: actually test this */
        char *bitstr = line + strpos(line, "bits=")+1;
        if (!strncmp(bitstr, "bits=", 5)) {
            int bits = 0;
            sscanf(bitstr, "bits=%d", &bits);
            printf("[ (!) ] %s gave %d bits in %s\n", user, bits, channel);
        }

        /* ctcp */
        if (!strncmp(msg, "VERSION", 9)) {
            puts("[ (!) ] ctcp version"); 
            send_raw(conn, 0, "NOTICE %s :VERSION socket.h\r\n", DEST);
        } else if (!strncmp(msg, "PING ", 6)) {
            puts("[ (!) ] ctcp ping"); 
            send_raw(conn, 0, "NOTICE %s :PING %u\r\n", DEST, 88);
        } else if (!strncmp(msg, "TIME", 6)) {
            puts("[ (!) ] ctcp time"); 
            send_raw(conn, 0, "NOTICE %s :TIME %u\r\n", DEST, (unsigned)time(NULL));
        }

        /* greeting if enabled */
        if (db_exists(db_entry(chandb, "settings", "greeting"))) {
            if (!db_exists(db_entry(chandb, "greeted", user))) {
                db_mkitem(db_entry(chandb, "greeted", user));
                char greeting_format[BUF_SIZE];
                strcpy(greeting_format, "PRIVMSG %s :");
                strcat(greeting_format, db_getstr(db_entry(chandb, "greeting_format")));
                strcat(greeting_format, "\r\n");
                send_raw(conn, 0, greeting_format, DEST, user);
            }
        }

        /* mail checking */
        if (db_exists(db_entry("db", "mail", user))) {
            if (!db_file(db_entry("db", "mail", user))) {
                db_del(db_entry("db", "mail", user));
            } else {
                send_raw(conn, 0, "PRIVMSG %s :%s: mail from %s: %s\r\n", DEST, user,
                        db_file(db_entry("db", "mail", user)), 
                        db_getstr(db_entry("db", "mail", user, 
                                db_file(db_entry("db", "mail", user)))));

                db_del(db_entry("db", "mail", user, 
                            db_file(db_entry("db", "mail", user))));
            }
        }

        /* afk checking */
        if (db_exists(db_entry(chandb, "afk", user))) {
            send_raw(conn, 0, "PRIVMSG %s :%s is no longer afk\r\n", DEST, user);
            db_del(db_entry(chandb, "afk", user));
        }

        /* commands 
         * TODO: add permission checking to command handler,
         */
        if (msg[0] == prefix) {
            printf("[ (!) ] '%c' prefixed command detected\n", prefix);
            strchr(msg, '\r')[0] = '\0';

            void (*cmd_func) (irc_conn *conn, char *msg, char *user, char *channel, char *chandb);
            for (int i = 0; i < 24; ++i) {
                if (!strncmp(msg+1, commands[i]->trigger, strlen(commands[i]->trigger))) {
                    printf("[ (!) ] command identified: commands[%d]: '%s'\n", 
                            i, commands[i]->trigger);
                    cmd_func = commands[i]->func;
                    cmd_func(conn, msg, user, channel, chandb);
                    break;
                }
            }
        } else if (!strncmp(msg, ".bots", 5)) {
            send_raw(conn, 0, "PRIVMSG %s :Reporting in! [C]\r\n", DEST);
        } 

        /* free db vars */
        free(chandb);
    } else if (!strncmp(msgtype, "USERNOTICE", 10)) { /* TODO: actually test this */
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
    }
}
