NAME = xol
SRC_FILES = main.c
CC_FLAGS = -g -std=c11 -Wall -Wextra -Wpedantic \
		   -Wno-pragma-once-outside-header \
		   -fsanitize=address
CC = clang

all: build

.PHONY: build
build:
	@${CC} ${SRC_FILES} ${CC_FLAGS} -o ${NAME}

.PHONY: clean
clean:
	@rm -rf ${NAME} ${NAME}.dSYM

.PHONY: cpp
cpp:
	@${CC} ${SRC_FILES} ${CC_FLAGS} -E

.PHONY: run
run: build
	@./${NAME}

.PHONY: format
format:
	clang-format -i *.c *.h
