CC = gcc
CFLAGS = -Wall -Wextra

SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/shell.c $(SRCDIR)/utils.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = simple_shell

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
