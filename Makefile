CC = gcc
CFLAGS = -Wall -g -Iinclude
SRC = src/main.c src/scheduler.c src/process.c src/resource.c src/interrupt.c src/memory.c src/utils.c
OBJ = $(SRC:.c=.o)
EXEC = sistema_simulado

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

