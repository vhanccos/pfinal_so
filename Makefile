CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread
LDFLAGS = -pthread

# Archivos fuente
SRCS = main.c src/memory.c src/process.c src/scheduler.c

# Nombre del ejecutable
TARGET = scheduler

# Objeto
OBJS = $(SRCS:.c=.o)

# Regla principal
all: $(TARGET)

# Enlazar el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compilar archivos fuente
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f $(OBJS) $(TARGET)

# Reconstruir todo
rebuild: clean all

.PHONY: all clean rebuild
