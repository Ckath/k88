#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include "../../utils/curl.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

#include "../../utils/strutils.h"

static INI *feels;
static CURL *curl = NULL;
static time_t started = 0;
static time_t finished = 0;
static bool store_index = 0;
static char lock_index[256] = "\0";

static void
json_item(char *dest, char *json, char *item, char *end)
{
	char *start = strstr(json, item);
	if (!start) {
		dest[0] = '\0';
		return;
	}

	start += strlen(item)+1;
	strncpy(dest, start, BUFSIZE-1);
	char *e = strstr(dest, end);
	if (e) {
		e[0] = '\0';
	}
	return;
}

static CURLcode
api_request(chunk *res, char *url)
{
	/* configure curl request */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wrcb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);

	/* make request */
	return curl_easy_perform(curl);
}

static void
parse_thread(char *board, char *thread, bool ws, char *json)
{
	/* id of post and board sector */
	char id[BUFSIZE];
	char board_sec[20];
	sprintf(board_sec, "%d_%s", !store_index, board);

	/* iterate over every post */
	char *json_ptr = json;
	while ((json_ptr = strstr(json_ptr+1, "\"no\""))) {
		json_item(id, json_ptr, "\"no\"", ",");
		if (!id) {
			break;
		}

		char post[BUFSIZE];
		json_item(post, json_ptr, "\"com\":", "\",\"");
		char *feel = post;
		int tfws = 0;
		while ((feel = strstr(feel+1, "&gt;tfw "))) {
			/* false hit on post without comment */
			if (strstr(json_ptr, "\"com\":") > strchr(json_ptr, '}')) {
				continue;
			}

			/* cleanup >tfw */
			char tfw[BUFSIZE];
			strcpy(tfw, feel);
			char *tfw_end = strstr(tfw, "<\\/");
			if (tfw_end) {
				tfw_end[0] = '\0';
			}
			strrplc(tfw, "&gt;", "3>");
			strrplc(tfw, "&lt;", "<");
			strrplc(tfw, "&#039;", "'");
			strrplc(tfw, "&quot;", "\"");
			strrplc(tfw, "&amp;", "&");
			strrplc(tfw, "&apos;", "'");
			strrplc(tfw, "<br>", "");
			strunescape(tfw);


			/* create urlindex and store */
			char url[100];
			sprintf(url, ws ? "https://boards.4channel.org/%s/thread/%s#p%s" :
					"https://boards.4chan.org/%s/thread/%s#p%s",
					board, thread, id);
			if (tfws++) { /* workaround keep a unique url index */
				char numbering[5];
				sprintf(numbering, " #%d", tfws);
				strcat(url, numbering);
			}
			sini_write(feels, board_sec, url, tfw);
		}
	}
}

static void
parse_board(char *board, bool ws)
{
	/* build url */
	char url[BUFSIZE];
	chunk res = { .memory = malloc(1), .size = 0 };
	sprintf(url, "https://a.4cdn.org/%s/threads.json", board);

	/* give up on curl error */
	CURLcode r = api_request(&res, url);
	if (r != CURLE_OK) {
		free(res.memory);
		log_err("curl error: %s\n", curl_easy_strerror(r));
		log_info("parse_board: trying to recovery by resetting started\n");
		started = 0;
		curl_reset();
		return;
	}

	/* iterate over every thread */
	char id[BUFSIZE];
	char *json_ptr = res.memory;
	while ((json_ptr = strstr(json_ptr+1, "\"no\""))) {
		json_item(id, json_ptr, "\"no\"", ",");
		if (!id[0]) {
			break;
		}

		/* call parse_thread with curl return data of thread api */
		sprintf(url, "https://a.4cdn.org/%s/thread/%s.json", board, id);
		chunk rres = { .memory = malloc(1), .size = 0 };
		r = api_request(&rres, url);
		if (r != CURLE_OK) {
			free(rres.memory);
			log_err("curl error: %s\n", curl_easy_strerror(r));
			log_info("parse_thread_call: trying to recovery by resetting started\n");
			started = 0;
			curl_reset();
			continue;
		}
		parse_thread(board, id, ws, rres.memory);
		free(rres.memory);
	}
	free(res.memory);
}

static void
update_cache()
{
	/* curl is stupid and breaks my sockets if I init it any sooner */
	if (!curl) {
		curl_init();
		curl = curl_easy_init();
	}

	/* set start time */
	started = time(NULL);
	char timestr[13];
	sprintf(timestr, "%lld", (long long) started);
	sini_write(feels, "updated", "started", timestr);

	/* iterate over boards from boards api return data */
	chunk res = { .memory = malloc(1), .size = 0 };
	CURLcode r = api_request(&res, "https://a.4cdn.org/boards.json");
	if (r != CURLE_OK) {
		log_err("curl error: %s\n", curl_easy_strerror(r));
		log_info("update_cache: trying to recovery by resetting started\n");
		started = 0;
		curl_reset();
	} else {
		char *json_ptr = res.memory;
		char board[BUFSIZE];
		char ws[BUFSIZE];
		while ((json_ptr = strstr(json_ptr+1, "\"board\""))) {
			json_item(board, json_ptr, "\"board\":", "\",\"");
			if (!board[0]) {
				break;
			}
			json_item(ws, json_ptr, "\"ws_board\"", ",");
			parse_board(board, (ws[0] == '1'));
		}
	}
	free(res.memory);

	/* mark new data index for usage */
	store_index = !store_index;
	char indexstr[2];
	sprintf(indexstr, "%d", store_index);
	sini_write(feels, "updated", "index", indexstr);

	/* wipe old data */
	char oldindex[3];
	sprintf(oldindex, "%d_", !store_index);
	char **boards = sini_list_sections(feels);
	for (int i = 0; boards[i]; ++i) {
		if (strstr(boards[i], oldindex)) {
			sini_remove(feels, boards[i], NULL);
		}
	}

	/* set end time */
	finished = time(NULL);
	sprintf(timestr, "%lld", (long long) finished);
	sini_write(feels, "updated", "finished", timestr);
	log_info("finished updating chanscraper cache\n");
}

static void
handle_timed(irc_conn *s, char *index, time_t time)
{
	/* hacked in gate to stop it triggering on every server */
	if (!lock_index[0]) {
		strcpy(lock_index ,index);
	} if (strcmp(lock_index, index)) {
		return;
	}

	if (s->init && time-started > 60*60*4/* 4 hours */) {
		log_info("updating chanscraper cache\n");
		update_cache();
	}
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (!strncmp(msg, "feelreveal", 10) || !strncmp(msg, "revealfeel", 10)) {
		char feelboard[256];
		strcpy(feelboard, mods_get_config(mi->index, "lastfeel"));
		char *feelboardp = strstr(feelboard, "org/");
		feelboardp += 3;
		strchr(feelboardp+1, '/')[0] = '\0';
		send_privmsg("%s/", feelboardp);
	} else if (!strncmp(msg, "feellast", 8) || !strncmp(msg, "lastfeel", 8)) {
		send_privmsg("%s", mods_get_config(mi->index, "lastfeel"));
	} else if (!strncmp(msg, "feel", 4)) {
		srand(time(NULL));
		char *board = strchr(msg, ' ');
		char board_sec[20];
		if (!board) {
			char **boardlist = sini_list_sections(feels);
			int boardcount = 0;
			while (boardlist[++boardcount]);
			strcpy(board_sec, boardlist[(rand()%(boardcount-1))+1]);
		} else {
			board+=1;
			sprintf(board_sec, "%d_%s", store_index, board);
			strrplc(board_sec, "/", "");
		}

		char **feellist = sini_list_items(feels, board_sec);
		if (!feellist) {
			send_privmsg("404 no feels found");
			return;
		}
		int feelcount = 0;
		while(feellist[++feelcount]);
		int feelpick = rand()%feelcount;
		send_privmsg("%s", ini_read(feels, board_sec, feellist[feelpick]));
		mods_set_config(mi->index, "lastfeel", feellist[feelpick]);
	} else if (!strncmp(msg, "findfeel", 8)) {
		/* check if theres something to search at all */
		char *search = strchr(msg, ' ');
		if (!search) {
			return;
		} else {
			search+=1;
		}

		/* bodge in a structure to hold feels and their origin */
		typedef struct tfw_struct{
			char tfw[BUFSIZE];
			char post[100];
		} tfw_struct;
		int results = 0;
		tfw_struct *tfws = NULL;

		/* collect list of results */
		char **boardlist = sini_list_sections(feels);
		for (int i = 1; boardlist[i]; ++i) {
			char **postlist = sini_list_items(feels, boardlist[i]);
			if (!postlist) {
				continue;
			}
			for (int ii = 0; postlist[ii]; ++ii) {
				char *post = ini_read(feels, boardlist[i], postlist[ii]);
				if (strstr(post, search)) {
					tfws = realloc(tfws, sizeof(tfw_struct)*++results);
					strcpy(tfws[results-1].tfw, post);
					strcpy(tfws[results-1].post, postlist[ii]);
				}
			}
		}

		/* pick a random one */
		if (results) {
			srand(time(NULL));
			int pick = rand()%results;
			send_privmsg("%s", tfws[pick].tfw);
			mods_set_config(mi->index, "lastfeel", tfws[pick].post);
			free(tfws);
		} else {
			send_privmsg("404 no feels found");
		}

	} else if (!strncmp(msg, "scrapedebug", 11)) {
		time_t now = time(NULL);
		if (started < finished) {
			send_privmsg("last updated: %dh %dm %ds ago, duration: %dm %ds",
					(now-started)/60/60, ((now-started)/60)%60, (now-started)%60,
					(finished-started)/60, (finished-started)%60);
		} else {
			send_privmsg("still updating, duration: %dm %ds",
					(now-started)/60, (now-started)%60);
		}
	} else if (mi->mod && !strncmp(msg, "scrapeupdate", 12)) {
		send_privmsg("updating cache again, this will take a while");
		log_info("updating chanscraper cache, manually triggered\n");
		update_cache();
	}
}

void
chanscraper_init()
{
	mods_new("chanscraper", true);
	mods_cmdmsg_handler(handle_cmdmsg);
	mods_timed_handler(handle_timed);

	/* restore past run data */
	feels = ini_load("mods/chanscraper/feels.ini");
	char *stored_start = ini_read(feels, "updated", "started");
	char *stored_finish = ini_read(feels, "updated", "finished");
	char *stored_index = ini_read(feels, "updated", "index");
	if (stored_start) {
		sscanf(stored_start, "%lld", (long long *) &started);
	} if (stored_finish) {
		sscanf(stored_finish, "%lld", (long long *) &finished);
	} if (stored_index) {
		sscanf(stored_index, "%d", &store_index);
	}
}
