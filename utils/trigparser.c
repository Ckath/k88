#include <string.h>
#include "../core/irc.h" /* for BUFSIZE */
#include "../core/modules.h" /* for msg_info */
#include "../utils/strutils.h"
#include "strutils.h"

ssize_t
parse_trig(char *trig, msg_info *mi, char *arg) {
	ssize_t parsed = 0;
	parsed += strrplc(trig, "{user}", mi->user);
	parsed += strrplc(trig, "{chan}", mi->chan);
	parsed += strrplc(trig, "{channel}", mi->chan);
	if (arg) {
		parsed += strrplc(trig, "{arg}", arg);
	}
	return parsed;
}
