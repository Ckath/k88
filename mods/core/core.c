#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <systemd/sd-daemon.h>

/* required */
#include "../modtape.h"
#include "../../core/irc.h"
#include "../../core/log.h"
#include "../../core/modules.h"

static char crash_data[BUFSIZE];
static char startupmsg[BUFSIZE] = { '\0' };
static time_t self_init;
static int mistimes = 0;
static uint64_t watchdog_interval = 0;
static bool systemd_watchdog;

static void
handle_timed(irc_conn *s, char *index, time_t t)
{
	if (s->init && t - s->heartbeat > 300) {
		s->heartbeat = t;
		log_err("connection timed out, resetting\n");
		reconnect_conn(s);
	}

	/* systemd watchdog */
	if (systemd_watchdog && !(t%(watchdog_interval/3))) {
		sd_notify(0, "WATCHDOG=1");
	}

	/* ideally given time should be current time */
	if (t != time(NULL)) {
		mistimes++;
	}
}

static void
handle_rawmsg(msg_info *mi, char *line)
{
	mi->conn->heartbeat = time(NULL);
	if (!strncmp(line, "PING ", 5)) {
		line[1] = 'O';
		send_raw(mi->conn, 0, line);
	} else if(!strncmp(line, "ERROR", 5)) {
		log_err("recieved ERROR, resetting connection\n");
		reconnect_conn(mi->conn);
	} else if (!mi->conn->init && strstr(line, " MODE ")) {
		join_chans(mi->conn, ini_read(mi->conn->globalconf,
					mi->conn->index, "chans"));
		if (startupmsg[0] &&
				!strncmp(mi->conn->index, startupmsg, strlen(mi->conn->index))) {
			send_raw(mi->conn, 0, strchr(startupmsg, ' ')+1);
			startupmsg[0] = '\0';
		}
		mi->conn->init = 1;
		mi->conn->init_time = time(NULL);
	}
}

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* IBIP */
	if (!strncmp(msg, ".bots", 5)) {
		send_privmsg("Reporting in! [C] fix shit: https://github.com/ckath/k88");
	}

	/* ctcp */
	if (!strncmp(msg, "VERSION", 9)) {
		log_info("ctcp version\n");
		send_notice("VERSION socket.h");
	} else if (!strncmp(msg, "PING ", 6)) {
		log_info("ctcp ping\n");
		send_notice("PING %u", 88);
	} else if (!strncmp(msg, "TIME", 6)) {
		log_info("ctcp time\n");
		send_notice("TIME %u", (unsigned)time(NULL));
	}

}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (!strncmp(msg, "status", 6)) {
		time_t now = time(NULL);
		struct timespec ts_now;
		clock_gettime(CLOCK_MONOTONIC_RAW, &ts_now);
		send_privmsg("bot uptime: %dh %dm, " \
				"connection uptime: %dh %dm, " \
				"reconnects: %d, " \
				"desyncs: %d, " \
				"internal lag: %.2fms, " \
				"last crash/restart reason: %s",
				(now-self_init)/60/60, ((now-self_init)/60)%60,
				(now-mi->conn->init_time)/60/60, ((now-mi->conn->init_time)/60)%60,
				mi->conn->reconns, mistimes,
				(ts_now.tv_sec-mi->ts.tv_sec)*1000+(double)(ts_now.tv_nsec-mi->ts.tv_nsec)/1000000,
				crash_data);
	}
}

void
core_init()
{
	/* retreive restart/crash info from file, if any */
	FILE *crash_file = NULL;
	if ((crash_file = fopen("/tmp/k88_crash", "r"))) {
		fgets(crash_data, sizeof(crash_data)-1, crash_file); 
		fclose(crash_file);
		remove("/tmp/k88_crash");
	} else {
		strcpy(crash_data, "unknown fault");
	}
	self_init = time(NULL);

	/* retrieve startup msg left on shutdown */
	FILE *death_file = NULL;
	if ((death_file = fopen("/tmp/k88_death", "r"))) {
		fgets(startupmsg, sizeof(startupmsg)-1, death_file);
		fclose(death_file);
		remove("/tmp/k88_death");
	}

	/* detect if under systemd watchdog */
	watchdog_interval = 0;
	systemd_watchdog = sd_watchdog_enabled(0, &watchdog_interval);
	if (systemd_watchdog) {
		setbuf(stdout, NULL); /* probably does something, in systemd example */
		watchdog_interval /= 1000000; /* convert usec(thx systemd) to secs */
		log_info("%d second systemd watchdog detected, notifying ready\n",
				watchdog_interval);
		sd_notify(0, "READY=1");
	}

	mods_new("core", true);
	mods_timed_handler(handle_timed);
	mods_rawmsg_handler(handle_rawmsg);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);
}
