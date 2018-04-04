CC=clang
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))
CFLAGS=

%.o: %.c
	clang-format -i -style=google $^
	$(CC) -c $< -o $@ $(CFLAGS)

default: server

server: $(OBJS) libz.a
	$(CC) $^ -o $@ $(CFLAGS) -pthread

clean:
	rm -f server *.o *.a *.zip

libz.a:
	@cd zlib && ./configure
	$(MAKE) -C zlib/ static && cp zlib/libz.a ./
	$(MAKE) -C zlib/ clean

zip:
	git archive --format=zip --prefix=comp4621/ --output=comp4621.zip HEAD
