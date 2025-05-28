.PHONY: debug clean release

SRCS = $(shell ls . | grep "\.c" | grep -v lib )

OUT = main.out

OUT_FOLDER = $(CMD)/bin

TESTS := src/history.h src/str.h 

CFLAGS = -g -Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion -Wformat=2

debug: lib.c $(SRCS) clean 
	@$(foreach f, $(SRCS), gcc $(CFLAGS) -g lib.c $(f) -o $(OUT_FOLDER)/$(f:.c=); )
	@ln -s $(OUT_FOLDER)/s $(OUT_FOLDER)/sv

release: clean
	@$(foreach f, $(SRCS), gcc $(CFLAGS) -O2 lib.c $(f) -o $(OUT_FOLDER)/$(f:.c=) ; )
	@ln -s $(OUT_FOLDER)/s $(OUT_FOLDER)/sv

clean:
	@$(foreach f, $(SRCS), rm -f $(OUT_FOLDER)/$(f:.c=); )
	@rm -f $(OUT_FOLDER)/sv

