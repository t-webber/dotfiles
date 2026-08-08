CFLAGS = -Wall -Wextra -pedantic
CFLAGS += -s -Ofast -march=native -mtune=native -flto
CFLAGS += -lX11 -lXft -I/usr/include/freetype2 -pthread
TARGET = $(BIN)/herbe

.PHONY: clean

$(TARGET): herbe.c
	cc $^ $(CFLAGS) -o $@

clean:
	rm -f $(TARGET)
