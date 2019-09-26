CC=gcc
CFLAGS= -g -Wall -Wextra
LIBS= -ldl

BINARIES= prog plugin.so

all: $(BINARIES)

prog: matrix_pl.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

plugin.so: plugin.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^

clean:
	rm -f $(BINARIES)

test:
	LD_LIBRARY_PATH="." ./prog plugin.so
