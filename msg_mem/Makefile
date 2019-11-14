CC = gcc
CFLAGS = -g 
LIBS = -lrt
ELF = mem_server mem_client

SRC1 = mem_server.c
OBJ1 = $(SRC1:%.c=%.o)

SRC2 = mem_client.c
OBJ2 = $(SRC2:%.c=%.o)

all:	${ELF}

mem_server: $(OBJ1)
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

mem_client: $(OBJ2)
	${CC} ${CFLAGS} -o $@ $^ ${LIBS}

$(OBJ1): %.o: %.c 
	${CC} ${CFLAGS} -c -o $@ $<

$(OBJ2): %.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f ${ELF} *.o *.d