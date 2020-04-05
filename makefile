NAME = k88
CFLAGS = -g
LIBS = -lpthread
SRC = ini/src/ini.c llist.c socks.c util.c nodb.c commands.c irc.c parse.c sockme.c
OBJ = ${SRC:.c=.o}
CC = gcc

.c.o:
	@echo CC -c $<
	@${CC} -c ${CFLAGS} $< ${LIBS} -o ${<:.c=.o}

${NAME}: ${SRC} ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${CFLAGS} ${OBJ} ${LIBS}

clean:
	@echo cleaning...
	@rm -f ${NAME} ${OBJ}

run: ${NAME}
	@echo launching...
	@./${NAME}
