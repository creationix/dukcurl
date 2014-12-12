CFLAGS=-Iduktape-releases/src $(shell curl-config --cflags)
LIBS=$(shell curl-config --libs)

ifeq ($(shell uname -s),Darwin)
  LIBS+= -bundle -undefined dynamic_lookup
  CFLAGS+= -fno-common
else
  LIBS+= -shared
  CFLAGS+= -pthread -fPIC
endif

all: curl.so

curl.so: dukcurl.c dukcurl.h
	$(CC) $< -o $@ ${CFLAGS} ${LIBS}

clean:
	rm -f curl.so

