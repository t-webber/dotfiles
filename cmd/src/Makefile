CC := gcc

LIBS := $(wildcard lib*.c)
LIB_OBJS := $(LIBS:.c=.o)

ZLIB_SRCS := $(wildcard zlib/*.c)
ZLIB_OBJS := $(ZLIB_SRCS:.c=.o)

SRCS := $(filter-out $(LIBS), $(wildcard *.c))

B := $(CMD)/bin

CFLAGS := -std=c99

CFLAGS += -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Werror -Wfatal-errors
CFLAGS += -Wfloat-equal -Wcast-align -Wcast-qual -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -Wold-style-definition -Wmissing-declarations -Wnested-externs -Wredundant-decls
CFLAGS += -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wpointer-arith
CFLAGS += -Wwrite-strings -Wunreachable-code -Wlong-long

ifeq ($(VAR),Linux)
CFLAGS += -Wlogical-op
endif

CFLAGS += -Wno-unused-command-line-argument

BINARIES := $(patsubst %.c, $B/%, $(SRCS))
LINKS := $B/v $B/rr $B/mc $B/mg $B/nd $B/rnc $B/cn $B/cs

.PRECIOUS: %.o
.PHONY: debug clean release build default

default: release

ZFLAGS := cast-qual cast-align switch-default sign-conversion shorten-64-to-32 format-nonliteral switch-enum long-long error=conversion conversion
ZFLAGS := $(filter-out $(addprefix -W, $(ZFLAGS)),$(CFLAGS)) $(addprefix -Wno-,$(ZFLAGS))

DEBUG_FLAGS := -O0 -g
RELEASE_FLAGS := -O3 -ffast-math -flto -s -march=native -mtune=native

debug: CFLAGS += $(DEBUG_FLAGS)
debug: ZFLAGS += $(DEBUG_FLAGS)
debug: clean build

release: CFLAGS += $(RELEASE_FLAGS)
release: ZFLAGS += $(RELEASE_FLAGS)
release: build

build: $B $(BINARIES) $(LINKS)
	@printf "" # hide "nothing to be done"

$B:
	@mkdir -p $B

$B/v: $B/s
	ln -sf $< $@

$B/rr: $B/r
	ln -sf $< $@

$B/nd: $B/n
	ln -sf $< $@

$B/rnc: $B/rn
	ln -sf $< $@

$B/mc $B/mg: $B/m
	ln -sf $< $@

$B/cn $B/cs: $B/c
	ln -sf $< $@

$B/m: m.o $(LIB_OBJS) $(ZLIB_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$B/%: %.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

zlib/%.o: zlib/%.c
	$(CC) $(ZFLAGS) $^ -c -o $@ -include unistd.h

clean:
	rm -f *.o *.a zlib/*.o zlib/*.a
	rm -rf $B
