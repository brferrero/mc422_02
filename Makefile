OUTPUT=corrida

CC=gcc

CC_OPT=-Wall -pedantic
CC_LIBS=-lpthread

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): $(OUTPUT).c
	$(CC) -o $(OUTPUT) $(CC_OPT) $(OUTPUT).c $(CC_LIBS)

.PHONY: clean
clean:
	rm -f $(OUTPUT)
