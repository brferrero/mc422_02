OUTPUT=corrida

CC=gcc

CC_OPT=-std=c11 -Wall -ansi -pedantic -D _BSD_SOURCE
CC_LIBS=-lpthread

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): $(OUTPUT).c
	$(CC) -o $(OUTPUT) $(CC_OPT) $(OUTPUT).c $(C_LIBS)

.PHONY: clean
clean:
	rm -f $(OUTPUT)
