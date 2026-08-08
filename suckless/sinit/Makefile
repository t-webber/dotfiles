CFLAGS = -Wall -Wextra -pedantic
CFLAGS += -s -Ofast -march=native -mtune=native -flto
TARGET = $(BIN)/init

$(TARGET): sinit.c Makefile
	sudo cc $< $(CFLAGS) -o $@
	sudo chmod ugoa+rwx $@
