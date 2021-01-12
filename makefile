NAME = k88

CC       ?= gcc
CPPFLAGS += -Imods/markov/chains/deps
CFLAGS   += -g -Os
LDFLAGS  += -lpthread -lssl -lcrypto -lcurl

SRC  = ${NAME}.c ini_rw/ini_rw.c mods/modtape.c
SRC += $(wildcard core/*.c utils/*.c mods/**/*.c)
SRC += mods/markov/chains/deps/hash/hash.c mods/markov/chains/src/chains.c 
OBJ  = ${SRC:.c=.o}

.PHONY: update_mods

.c.o:
	@echo CC -c $<
	@${CC} -c ${CFLAGS} ${CPPFLAGS} $< -o ${<:.c=.o}

${NAME}: update_mods ${SRC} ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${CFLAGS} ${OBJ} ${LDFLAGS}

update_mods:
	@./modcodegen.sh

clean:
	@echo cleaning...
	@rm -f ${NAME} ${OBJ} mods/modtape.*

run: ${NAME}
	@echo launching...
	@./${NAME}
