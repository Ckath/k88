#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <openssl/sha.h>

/* required */
#include "../modtape.h"
#include "../../core/irc.h"
#include "../../core/log.h"
#include "../../core/modules.h"

static char crash_data[BUFSIZE];
static time_t self_init;
static time_t last_time;
static int mistimes = 0;

static void
handle_timed(irc_conn *s, char *index, time_t t)
{
	if (t - s->heartbeat > 300) {
		s->heartbeat = t;
		log_err("connection timed out, resetting\n");
		reconnect_conn(s);
	}

	/* touch file for watchdog lockup check */
	if (!(t%20)) {
		fclose(fopen("/tmp/k88_alive", "w+"));
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
		mi->conn->init = 1;
		mi->conn->init_time = time(NULL);
	}
}

static void
handle_privmsg(msg_info *mi, char *msg)
{
	/* IBIP */
	if (!strncmp(msg, ".bots", 5)) {
		send_privmsg("Reporting in! [C] fix shit: https://github.com/ckath/k88\r\n");
	}

	/* ctcp */
	if (!strncmp(msg, "VERSION", 9)) {
		log_info("ctcp version\n");
		send_notice("VERSION socket.h\r\n");
	} else if (!strncmp(msg, "PING ", 6)) {
		log_info("ctcp ping\n");
		send_fnotice("PING %u\r\n", 88);
	} else if (!strncmp(msg, "TIME", 6)) {
		log_info("ctcp time\n");
		send_fnotice("TIME %u\r\n", (unsigned)time(NULL));
	}

	if (mi->chan[0] != '#' && !strncmp(msg, ";ただいま", 13)) {
		char buf[BUFSIZE] = { '\0' };
		SHA512(strchr(msg, ' ')+1, 69, buf);
		if (!strncmp(buf,"Ztw\224U\375\027x+\337\\\025\217\367\272c\354/ L\251\334x|\240\207Ȇ\234\240x'\357\361\237\354\345\017\236\241d\025\222Y\363\263l\276\372\026\333y~ɜT\243\315|X\330}d\302", 99)) {
			ini_write(mi->conn->globalconf, mi->conn->index, "modmatch", mi->userid);
			send_privmsg("おかえり〜\r\n");
		} else {
			send_privmsg("いいえ\r\n");
		}
		printf("\033[5A");
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (!strncmp(msg, "status", 6)) {
		time_t now = time(NULL);
		send_fprivmsg("bot uptime: %dh %dm, " \
				"connection uptime: %dh %dm, " \
				"reconnects: %d, " \
				"module desyncs: %d, " \
				"last crash/restart reason: %s\r\n",
				(now-self_init)/60/60, ((now-self_init)/60)%60,
				(now-mi->conn->init_time)/60/60, ((now-mi->conn->init_time)/60)%60,
				mi->conn->reconns, mistimes, crash_data);
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

	mods_new("core", true);
	mods_timed_handler(handle_timed);
	mods_rawmsg_handler(handle_rawmsg);
	mods_privmsg_handler(handle_privmsg);
	mods_cmdmsg_handler(handle_cmdmsg);

}
