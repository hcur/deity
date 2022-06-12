NAME = deity

CC = gcc
CFLAGS += -lX11

PREFIX ?= /usr/local
BINDIR ?= ${PREFIX}/bin


${NAME}: ${NAME}.c
	$(CC) ${CFLAGS} -o ${NAME} ${NAME}.c

install:
	echo installing ${NAME} executable to ${DESTDIR}${PREFIX}${BINDIR}
	install -Dm775 ${NAME} ${DESTDIR}${PREFIX}${BINDIR}

uninstall:
	echo removing ${NAME} executable from ${DESTDIR}${PREFIX}${BINDIR}
	rm -f ${DESTDIR}${BINDIR}

clean:
	echo cleaning
	rm -fv ${NAME} ${WMNAME}.o

.PHONY: clean install uninstall
