NAME = k88
CFLAGS = -g -Os
LIBS = -lpthread -lssl -lcrypto -lcurl -Imods/markov/chains/deps
SRC = ${NAME}.c ini_rw/ini_rw.c mods/modtape.c
SRC += $(wildcard core/*.c utils/*.c mods/**/*.c)
SRC += mods/markov/chains/deps/hash/hash.c mods/markov/chains/src/chains.c 
OBJ = ${SRC:.c=.o}
CC = gcc
.PHONY: update_mods

.c.o:
	@echo CC -c $<
	@${CC} -c ${CFLAGS} $< ${LIBS} -o ${<:.c=.o}

${NAME}: update_mods ${SRC} ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${CFLAGS} ${OBJ} ${LIBS}

update_mods:
	@./modcodegen.sh

clean:
	@echo cleaning...
	@rm -f ${NAME} ${OBJ} mods/modtape.*

run: ${NAME}
	@echo launching...
	@./${NAME}
