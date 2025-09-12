LIBS := lib.c libfiles.c libsort.c libexec.c
LIB_OBJS := $(LIBS:.c=.o)

ZLIB_SRCS := $(wildcard zlib/*.c)
ZLIB_OBJS := $(ZLIB_SRCS:.c=.o)

SRCS := $(filter-out $(LIBS), $(wildcard *.c))

B := $(CMD)/bin

CFLAGS += -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Werror
CFLAGS += -Wfloat-equal -Wcast-align -Wcast-qual -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -Wold-style-definition -Wmissing-declarations -Wnested-externs -Wredundant-decls
CFLAGS += -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wpointer-arith
CFLAGS += -Wwrite-strings -Wunreachable-code -Wlogical-op -Wno-unused-parameter

BINARIES := $(patsubst %.c, $B/%, $(SRCS))
LINKS := $B/sv $B/rr $B/mc $B/mg $B/nd $B/rnc

.PRECIOUS: %.o
.PHONY: debug clean release build

all: release

debug: CFLAGS = -O0 -g
debug: clean build

release: CFLAGS = -O3 -ffast-math -flto -s -march=native -mtune=native
release: build

build: $B $(BINARIES) $(LINKS)
	@echo -n "" # hide "nothing to be done"

$B:
	@mkdir -p $B

$B/sv: $B/s
	ln -sf $< $@

$B/rr: $B/r
	ln -sf $< $@

$B/nd: $B/n
	ln -sf $< $@

$B/rnc: $B/rn
	ln -sf $< $@

$B/mc $B/mg: $B/m
	ln -sf $< $@

$B/m: m.o $(LIB_OBJS) $(ZLIB_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$B/%: %.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

zlib/%.o: zlib/%.c
	$(CC) $(CFLAGS) $^ -c -o $@ -include unistd.h

clean:
	rm -f *.o *.a zlib/*.o zlib/*.a
	rm -rf $B
