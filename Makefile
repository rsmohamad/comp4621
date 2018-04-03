CC=clang

default: server

server: main.c
	$(CC) $< -o $@ -pthread

clean:
	rm -r server
