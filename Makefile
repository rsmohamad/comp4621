CC=clang
SRCS=$(wildcard *.c)
HEAD=$(wildcard *.h)
OBJS=$(patsubst %.c, %.o, $(SRCS))
CFLAGS=

%.o: %.c
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

format:
	clang-format -i -style=google $(HEAD) $(SRCS)
