LIBS := lib.c libfiles.c libsort.c
LIB_OBJS := $(LIBS:.c=.o)

SRCS := $(filter-out $(LIBS), $(wildcard *.c))

BIN := $(CMD)/bin

CFLAGS += -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Werror
CFLAGS += -Wfloat-equal -Wcast-align -Wcast-qual -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -Wold-style-definition -Wmissing-declarations -Wnested-externs -Wredundant-decls
CFLAGS += -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wpointer-arith
CFLAGS += -Wwrite-strings -Wunreachable-code -Wlogical-op -Wno-unused-parameter

BINARIES := $(patsubst %.c, $(BIN)/%, $(SRCS))
LINKS := $(BIN)/sv $(BIN)/rv $(BIN)/mc

# .SILENT:
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

$(BIN)/m: m.o $(LIB_OBJS) libz.a
	$(CC) $(CFLAGS) $(LDFLAGS) -Izlib $^ libz.a -o $@

$(BIN)/%: %.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@


LIBZ_OBJS := adler32.o crc32.o deflate.o infback.o inffast.o inflate.o inftrees.o trees.o zutil.o compress.o uncompr.o gzclose.o gzlib.o gzread.o gzwrite.o
LIBZ_OBJS := $(foreach f, $(LIBZ_OBJS), zlib/$f)
AR=ar
ARFLAGS=rc

libz.a: $(LIBZ_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBZ_OBJS)


clean: 
	rm -f *.o *.a
	rm -rf $(BIN)
	mkdir -p $(BIN)
	touch $(BIN)/.gitkeep

