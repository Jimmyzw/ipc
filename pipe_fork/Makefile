ELF = pipe_test

SRC = $(shell find -name '*.c')
OBJ = $(SRC:%.c=%.o)

$(ELF): $(OBJ)
	gcc -o $@ $^

$(OBJ): %.o: %.c
	gcc -c -o $@ $<

.PHONY: clean
clean:
	$(RM) $(OBJ)
	$(RM) $(ELF)