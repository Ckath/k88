NAME = k88_32
CFLAGS = -m32
SRC = llist.c nodb.c sockme.c socks.c util.c
OBJ = ${SRC:.c=.o}
CC = gcc

.c.o:
	@echo CC -c $<
	@${CC} -c ${CFLAGS} $<

${NAME}: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${CFLAGS} ${OBJ}

clean:
	@echo cleaning...
	@rm -f ${NAME} ${OBJ}

run: ${NAME}
	@echo launching...
	@./${NAME}
