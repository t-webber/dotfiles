INCS  = -I. -I/usr/include 
INCS += $(shell pkg-config --cflags xft)
LIBS  = -L/usr/lib -lc -lcrypt -lX11 -lXext -lXrandr
LIBS += $(shell pkg-config --libs xft)

CFLAGS = -std=c99 -pedantic -Wall
CFLAGS += $(INCS) $(LIBS)
CFLAGS += -s -Ofast -march=native -mtune=native -flto

TARGET := $(BIN)/slock

.PHONY: clean

$(TARGET): slock.c $(BIN)
	$(CC) $(CFLAGS) $< -o $@
	sudo chown root:root $@
	sudo chmod 755 $@
	sudo chmod u+s $@

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -f $(TARGET)
