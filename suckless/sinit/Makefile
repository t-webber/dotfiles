CFLAGS = -Wall -Wextra -pedantic
CFLAGS += -s -Ofast -march=native -mtune=native -flto
TARGET = chaos

.PHONY: clean

$(TARGET): sinit.c
	sudo cc $^ $(CFLAGS) -o $@

clean:
	rm -f $(TARGET)
