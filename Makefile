.phony = debug test-gdb release test clean
all: debug

# common compiler options
CC = gcc
LD = gcc
COMMON_CFLAGS = -std=gnu2x -Wall -Wextra -Wno-unused-function -I./src
COMMON_LDFLAGS = 

# rules for building the library itself
LIBRARY = libgrapher.so
LIB_SRC = $(shell find ./src -name "*.c")
LIB_OBJ = $(LIB_SRC:.c=.o)
LIB_CFLAGS = -fpic -fvisibility=hidden
LIB_LDFLAGS = -shared -lm

# should it be run in gdb
GDB = 

test-gdb: GDB += gdb
test-gdb: test

debug: COMMON_CFLAGS += -DDEBUG -g
debug: COMMON_LDFLAGS += -g
debug: release

release: $(LIBRARY)

$(LIBRARY): $(LIB_OBJ)
	$(LD) $(LIB_OBJ) $(COMMON_LDFLAGS) $(LIB_LDFLAGS) -o $(LIBRARY)

$(LIB_OBJ): %o: %c
	$(CC) $(COMMON_CFLAGS) $(LIB_CFLAGS) -c $^ -o $@

# rules for building and running the tester
TESTER = test/tester
TEST_SRC = $(shell find ./test -name "*.c")
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_CFLAGS =
TEST_LDFLAGS = -ldl

test: $(TESTER) $(LIBRARY)
	LD_LIBRARY_PATH=. $(GDB) $(TESTER)

$(TESTER): $(TEST_OBJ)
	$(LD) $(TEST_OBJ) $(COMMON_LDFLAGS) $(TEST_LDFLAGS) -o $(TESTER)

$(TEST_OBJ): %o: %c
	$(CC) $(COMMON_CFLAGS) $(TEST_CFLAGS) -c $^ -o $@

# cleans up object files
clean:
	rm -f $(LIB_OBJ) $(TEST_OBJ) $(TESTER)
