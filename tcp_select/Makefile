CC = gcc
CFLAGS = -g
LIBS = 
ELF = tcpcli01 tcpservselect01

SRC1 = tcpcli01.c strcliselect01.c
OBJ1 = $(SRC1:%.c=%.o)

SRC2 = tcpservselect01.c
OBJ2 = $(SRC2:%.c=%.o)

all:	${ELF}

tcpcli01: $(OBJ1)
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

tcpservselect01: $(OBJ2)
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

$(OBJ1): %.o: %.c 
	${CC} ${CFLAGS} -c -o $@ $<

$(OBJ2): %.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f ${ELF} *.o *.d