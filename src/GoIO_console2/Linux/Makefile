SOURCES = main.c ngisensorworld.c ngideviceinterface.c ngidevice.c ngisensor.c
OBJS    = ${SOURCES:.c=.o}
CFLAGS  = `pkg-config gtk+-2.0 glib-2.0 gtkextra-2.0 GoIO --cflags`
LDADD   = `pkg-config gtk+-2.0 glib-2.0 gtkextra-2.0 GoIO --libs`
CC      = gcc
PACKAGE = goioconsole2

all : ${OBJS}
	${CC} -o ${PACKAGE} ${OBJS} ${LDADD}

.c.o:
	${CC} ${CFLAGS} -c $<

clean:
	-rm ${OBJS}
	-rm ${PACKAGE}
