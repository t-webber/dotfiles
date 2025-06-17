LIBS := lib.c libfiles.c
LIB_OBJS := $(LIBS:.c=.o)

SRCS := $(filter-out $(LIBS), $(wildcard *.c))


OUT_FOLDER := $(CMD)/bin

CFLAGS := -Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion -Wformat=2

BINARIES := $(patsubst %.c, $(OUT_FOLDER)/%, $(SRCS))
LINKS := $(OUT_FOLDER)/sv $(OUT_FOLDER)/rv $(OUT_FOLDER)/mc

.SILENT:
.PHONY: debug clean release

all: release 

debug: CFLAGS += -g
debug: clean build

release: CFLAGS += -O3
release: build

build: $(BINARIES) $(LINKS)

$(OUT_FOLDER)/sv: s.c
	ln -sf $(OUT_FOLDER)/s $@

$(OUT_FOLDER)/rv: r.c
	ln -sf $(OUT_FOLDER)/r $@

$(OUT_FOLDER)/mc: m.c
	ln -sf $(OUT_FOLDER)/m $@

$(OUT_FOLDER)/m: CFLAGS += -lz

$(OUT_FOLDER)/%: $(LIB_OBJS) %.c 
	mkdir -p $(OUT_FOLDER)
	gcc $(CFLAGS) $^ -o $@

clean: 
	rm $(LIB_OBJS)
	rm -rf $(OUT_FOLDER)
	mkdir -p $(OUT_FOLDER)
	touch $(OUT_FOLDER)/.gitkeep

