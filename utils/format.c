#include <string.h>
#include "../core/irc.h" /* for BUFSIZE */
#include "../core/modules.h" /* for msg_info */
#include "../utils/strutils.h"
#include "strutils.h"

ssize_t
mi_format(char *src, msg_info *mi, char *arg) {
	ssize_t parsed = 0;
	parsed += strrplc(src, "{user}", mi->user);
	parsed += strrplc(src, "{chan}", mi->chan);
	parsed += strrplc(src, "{channel}", mi->chan);
	if (arg) {
		parsed += strrplc(src, "{arg}", arg);
	} else {
		parsed += strrplc(src, "{arg}", mi->user);
	}
	return parsed;
}
