LIBS := lib.c libfiles.c libsort.c
LIB_OBJS := $(LIBS:.c=.o)

ZLIB_SRCS := $(wildcard zlib/*.c)
ZLIB_OBJS := $(ZLIB_SRCS:.c=.o)

SRCS := $(filter-out $(LIBS), $(wildcard *.c))

BIN := $(CMD)/bin

CFLAGS += -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 
CFLAGS += -Wfloat-equal -Wcast-align -Wcast-qual -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -Wold-style-definition -Wmissing-declarations -Wnested-externs -Wredundant-decls
CFLAGS += -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wpointer-arith
CFLAGS += -Wwrite-strings -Wunreachable-code -Wlogical-op -Wno-unused-parameter

BINARIES := $(patsubst %.c, $(BIN)/%, $(SRCS))
LINKS := $(BIN)/sv $(BIN)/rv $(BIN)/mc

.SILENT:
.PHONY: debug clean release out_folder build

all: release 

debug: CFLAGS += -g
debug: clean build

release: CFLAGS += -Ofast
release: build

build: out_folder $(BINARIES) $(LINKS)

out_folder:
	mkdir -p $(BIN)

$(BIN)/sv: $(BIN)/s
	ln -sf $< $@

$(BIN)/rv: $(BIN)/r
	ln -sf $< $@

$(BIN)/mc: $(BIN)/m
	ln -sf $< $@

$(BIN)/m: m.o $(LIB_OBJS) $(ZLIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BIN)/%: %.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

zlib/%.o: zlib/%.c
	$(CC) -Ofast $^ -c -o $@ -include unistd.h

clean:
	rm -f *.o *.a zlib/*.o zlib/*.a
	rm -rf $(BIN)
	mkdir -p $(BIN)
	touch $(BIN)/.gitkeep

