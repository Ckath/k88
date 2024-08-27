#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* required */
#include "../modtape.h"
#include "../../core/irc.h"
#include "../../core/log.h"
#include "../../core/modules.h"

static INI *reminders;
static time_t *times = NULL;

static void
update_times()
{
	/* obtain sections */
	char **remind_times = sini_list_sections(reminders);
	if (!remind_times) {
		if (times) {
			free(times);
			times = NULL;
		}
		return;
	}

	/* realloc times */
	int size;
	for (size = 0; remind_times[size]; ++size);
	times = realloc(times, sizeof(time_t)*(size+1));
	times[size] = 0;

	/* fill times */
	for (int i = 0; i < size; ++i) {
		char time[42];
		strcpy(time, strchr(remind_times[i], '_')+1);
		times[i] = strtoul(time, NULL, 0);
	}
}

static void
handle_timed(irc_conn *s, char *index, time_t t)
{
	/* bail checking without reminders */
	if (!times || !times[0] || !s->init) {
		return;
	}

	/* go over all remind times, popping messages on < now */
	time_t *r = times;
	bool time_popped = false;
	while (*r) {
		if (*r < t) {
			char sector[80];
			sprintf(sector, "%s_%u", s->index, *r);
			char **msgs = sini_list_items(reminders, sector);

			if (msgs) {
				/* send all messages for this time */
				for (int m = 0; msgs[m]; ++m) {
					send_raw(s, 0, "PRIVMSG %s\r\n",
							ini_read(reminders, sector, msgs[m]));
					sini_remove(reminders, sector, msgs[m]);
				}
				time_popped = true;
			}
		}
		r++;
	}

	if (time_popped) {
		update_times();
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (!strncmp(msg, "in ", 3) || !strncmp(msg, "remind ", 7)) {
		char *when = strchr(msg, ' '); /* make sure its worth while */
		if (when) {
			when++;
		} else {
			return;
		}
		time_t now = time(NULL);

		/* pretty bad way to get the amount here but I just dont care */
		unsigned amount = 0;
		sscanf(when, "%u", &amount); 
		if (!amount) {
			return;
		}

		/* janky unit char obtaining */
		char u = (when + (amount >= 100000000 ? 9 :
			amount >= 10000000 ? 8 :
			amount >= 1000000 ? 7 :
			amount >= 100000 ? 6 :
			amount >= 10000 ? 5 :
			amount >= 1000 ? 4 :
			amount >= 100 ? 3 :
			amount >= 10 ? 2 : 1))[0];

		/* unit word after space */
		char *unit = strchr(when, ' ');
		if (unit) {
			unit++;
		} else {
			return;
		}

		/* seconds, minutes, hours, days, weeks, months, years */
		char timestr[80];
		if (u == 'm' || !strncmp(unit, "min", 3)) {
			sprintf(timestr, amount == 1  ? "%d minute ago" :
					"%d minutes ago", amount);
			amount *= 60;
		} else if (u == 'h' || !strncmp(unit, "hour", 4)) {
			sprintf(timestr, amount == 1  ? "%d hour ago" :
					"%d hours ago", amount);
			amount *= 3600;
		} else if (u == 'd' || !strncmp(unit, "day", 3)) {
			sprintf(timestr, amount == 1  ? "%d day ago" :
					"%d days ago", amount);
			amount *= 86400;
		} else if (u == 'w' || !strncmp(unit, "week", 4)) {
			sprintf(timestr, amount == 1  ? "%d week ago" :
					"%d weeks ago", amount);
			amount *= 604800;
		} else if (!strncmp(unit, "month", 4)) {
			sprintf(timestr, amount == 1  ? "%d month ago" :
					"%d months ago", amount);
			amount *= 2680000;
		} else if (u == 'y' || !strncmp(unit, "year", 4)) {
			sprintf(timestr, amount == 1  ? "%d year ago" :
					"%d years ago", amount);
			amount *= 31500000;
		} else {
			sprintf(timestr, amount == 1  ? "%d second ago" :
					"%d seconds ago", amount);
		}

		char *remind;
		if (u == ' ') {
			remind = strchr(unit, ' ');
			if (remind) {
				remind++;
			} else {
				return;
			}
		} else {
			remind = unit;
		}

		char reminder[BUFSIZE];
		char index[100];
		char sector[80];
		sprintf(index, "%s_%s_%u", mi->chan+1, mi->user, now);
		sprintf(reminder, "%s :%s: %s (%s)", mi->chan, mi->user, remind, timestr);
		sprintf(sector, "%s_%u", mi->conn->index, now+amount);
		sini_write(reminders, sector, index, reminder);
		update_times();

		send_privmsg("ETA: %u seconds(%u)", amount, now+amount);
	} else if (!strncmp(msg, "on ", 3)) {
		char *when = strchr(msg, ' '); /* make sure its worth while */
		if (when) {
			when++;
		} else {
			return;
		}
		time_t now = time(NULL);

		/* pretty bad way to get the amount here but I just dont care */
		unsigned time = 0;
		sscanf(when, "%u", &time); 
		if (!time) {
			return;
		}

		char *remind = strchr(when, ' ');
		if (remind) {
			remind++;
		} else {
			return;
		}

		char reminder[BUFSIZE];
		char index[100];
		char timestr[80];
		char sector[80];
		strftime(timestr, 80, "%X %x %Z", localtime(&now));
		sprintf(index, "%s_%s_%u", mi->chan+1, mi->user, now);
		sprintf(reminder, "%s :%s: %s (%s)", mi->chan, mi->user, remind, timestr);
		sprintf(sector, "%s_%u", mi->conn->index, time);
		sini_write(reminders, sector, index, reminder);
		update_times();
		send_privmsg("ETA: %u seconds(%u)", time-now, time);
	}
}

void
remind_init()
{
	reminders = ini_load("mods/remind/reminders.ini");
	update_times();
	mods_new("remind", true);
	mods_timed_handler(handle_timed);
	mods_cmdmsg_handler(handle_cmdmsg);

}
