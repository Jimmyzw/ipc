ELF = pipe_test

SRC = $(shell find -name '*.c')
OBJ = $(SRC:%.c=%.o)

$(ELF): $(OBJ)
	gcc -g -o $@ $^

$(OBJ): %.o: %.c
	gcc -g -c -o $@ $<

.PHONY: clean
clean:
	$(RM) $(OBJ)
	$(RM) $(ELF)
