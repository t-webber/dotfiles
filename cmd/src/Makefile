LINKS := v rr mc mg nd rc cn cs lw lc lr

C ?= gcc

LIBS := $(wildcard lib*.c)
LIB_OBJS := $(LIBS:.c=.o)

ZLIB_SRCS := $(wildcard zlib/*.c)
ZLIB_OBJS := $(ZLIB_SRCS:.c=.o)

SRCS := $(filter-out $(LIBS), $(wildcard *.c))

B := $(CMD)/bin

CFLAGS := -std=c99

CFLAGS += -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 
CFLAGS += -Wfloat-equal -Wcast-align -Wcast-qual -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -Wold-style-definition -Wmissing-declarations -Wnested-externs -Wredundant-decls
CFLAGS += -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wpointer-arith
CFLAGS += -Wwrite-strings -Wunreachable-code -Wlong-long
CFLAGS += -Wdouble-promotion -Wformat-signedness -Wformat-overflow=2 -Wformat-truncation=2 -Wstringop-overflow=4 -Wstringop-truncation -Walloca -Walloc-size -Wvla -Wvla-larger-than=0 -Wunsafe-loop-optimizations
CFLAGS += -Woverflow -Wstrict-overflow=5 -Wsign-conversion -Wsign-compare -Wshift-overflow=2 -Wshift-negative-value
CFLAGS += -Wdangling-pointer -Wpointer-to-int-cast -Warray-bounds=2 -Wrestrict
CFLAGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches -Wmisleading-indentation -Werror=implicit-fallthrough -Waggressive-loop-optimizations -Wunused-but-set-variable 
CFLAGS += -Wno-common -Wstrict-aliasing -Wunused-value 
CFLAGS += -Wsuggest-attribute=const -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn
CFLAGS += -Werror -Wfatal-errors

BINARIES := $(patsubst %.c, $B/%, $(SRCS))
LINKS := $(addprefix $B/, $(LINKS))

DEBUG_FLAGS := -O0 -g
RELEASE_FLAGS := -O3 -ffast-math -flto 

ifeq ($(shell uname),Linux)
RELEASE_FLAGS += -march=native -mtune=native
else
#RELEASE_FLAGS += -mcpu=apple-m4
CFLAGS += -Wno-error=suggest-attribute=pure
endif

# ifeq ($(shell uname),Linux)
# RELEASE_FLAGS += -s
# CFLAGS += -Wformat-overflow=2 -Wformat-truncation=2 -Wstringop-overflow=4 -Wstringop-truncation 
# endif

OUT_NO := cast-qual cast-align switch-default sign-conversion shorten-64-to-32 format-nonliteral switch-enum long-long error=conversion conversion format-truncation suggest-attribute=pure suggest-attribute=const strict-overflow
ZFLAGS := $(filter-out $(addprefix -W, $(OUT_NO)),$(CFLAGS)) $(addprefix -Wno-,$(OUT_NO))

.PRECIOUS: %.o
.PHONY: debug clean release build default

default: release

debug: CFLAGS += $(DEBUG_FLAGS)
debug: ZFLAGS += $(DEBUG_FLAGS)
debug: build

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

$B/lw $B/lr $B/lc: $B/l
	ln -sf $< $@

$B/nd: $B/n
	ln -sf $< $@

$B/rc: $B/r
	ln -sf $< $@

$B/mc $B/mg: $B/m
	ln -sf $< $@

$B/cn $B/cs: $B/c
	ln -sf $< $@

$B/m: m.o $(LIB_OBJS) $(ZLIB_OBJS)
	$(CC) $(ZFLAGS) $^ -o $@

$B/%: %.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

zlib/%.o: zlib/%.c
	$(CC) $(ZFLAGS) $^ -c -o $@ -include unistd.h

m.o: m.c
	$(CC) $(ZFLAGS) $^ -c -o $@

clean:
	rm -f *.o *.a zlib/*.o zlib/*.a
	rm -rf $B
