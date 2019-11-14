CC = gcc
CFLAGS = -g 
LIBS = -lrt
ELF = mq_server mq_client

SRC1 = mq_server.c
OBJ1 = $(SRC1:%.c=%.o)

SRC2 = mq_client.c
OBJ2 = $(SRC2:%.c=%.o)

all:	${ELF}

mq_server: $(OBJ1)
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

mq_client: $(OBJ2)
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

$(OBJ1): %.o: %.c 
	${CC} ${CFLAGS} -c -o $@ $<

$(OBJ2): %.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f ${ELF} *.o *.d