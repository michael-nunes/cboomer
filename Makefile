SAFE_INSTALL_DIR=$(patsubst %/,%, "${DESTDIR}${PREFIX}")
LDFLAGS+=-lX11 -lGLX -lGL -lm
CFLAGS+=-Wall -Werror -Wno-unused-function

all: cboomer

run: cboomer
	./cboomer

cboomer: main.c option.h 
	$(CC) -o cboomer main.c ${CFLAGS} ${LDFLAGS}

build: clean cboomer

install: cboomer
	mkdir -p "${SAFE_INSTALL_DIR}"
	cp -v cboomer "${SAFE_INSTALL_DIR}"

uninstall:
	rm -v "${SAFE_INSTALL_DIR}/cboomer"

clean:
	rm -f cboomer

.PHONY: all install clean run uninstall build
