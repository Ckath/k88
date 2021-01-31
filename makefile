NAME = k88

CC       ?= gcc
CPPFLAGS += -Imods/markov/chains/deps
CFLAGS   += -g -Os
LDFLAGS  += -lsystemd -lpthread -lssl -lcrypto -lcurl

SRC  = ${NAME}.c ini_rw/ini_rw.c mods/modtape.c
SRC += $(wildcard core/*.c utils/*.c mods/**/*.c)
SRC += mods/markov/chains/deps/hash/hash.c mods/markov/chains/src/chains.c 
OBJ  = ${SRC:.c=.o}

.PHONY: update_mods gen_service

.c.o:
	@echo CC -c $<
	@${CC} -c ${CFLAGS} ${CPPFLAGS} $< -o ${<:.c=.o}

${NAME}: update_mods k88.service ${SRC} ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${CFLAGS} ${OBJ} ${LDFLAGS}

update_mods:
	@./modcodegen.sh

k88.service:
	@cp k88_servicetemplate k88.service
	@sed -i "s:PWD:`pwd`:g" k88.service
	@echo service file generated

clean:
	@echo cleaning...
	@rm -f ${NAME} ${OBJ} mods/modtape.*

run: ${NAME}
	@echo launching...
	@./${NAME}
