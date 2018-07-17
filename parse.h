#ifndef PARSE_H
#define PARSE_H

#include "extern.h"
#include "irc.h"
#include "commands.h"
#include "util.h"
#include "nodb.h"

void handle_raw(irc_conn *conn, char *line);

#endif
