#ifndef LOG_H
#define LOG_H

/* yes, log formatting is only prepending colored identifiers to printfs */
#define log_recv(logformat, ...) printf("[ \033[34m<<<\033[39m ] " logformat, ##__VA_ARGS__)
#define log_send(logformat, ...) printf("[ \033[32m>>>\033[39m ] " logformat, ##__VA_ARGS__)
#define log_info(logformat, ...) printf("[ \033[33m(!)\033[39m ] " logformat, ##__VA_ARGS__)
#define log_err(logformat, ...) fprintf(stderr, "[ \033[31m!!!\033[39m ] " logformat, ##__VA_ARGS__)

#endif
