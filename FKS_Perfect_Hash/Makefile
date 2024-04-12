CC=gcc
CFLAGS=-Wpedantic -Wall -Wextra -Werror -Wvla -std=c11 -g

BUILD_DIR = ./build

SRC_DIR = src
SRC = fks_level1.c fks_level2.c hash_chaining.c hash_parameters.c hash_func.c
SRC := $(addprefix $(SRC_DIR)/,$(SRC))

STATIC_OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o-static)
SHARED_OBJS = $(SRC:%.c=$(BUILD_DIR)/%.o-shared)

STATIC_LIB = $(BUILD_DIR)/libhash.a
SHARED_LIB = $(BUILD_DIR)/libhash.so

BIN = $(BUILD_DIR)/test_fks
STATIC_BIN = $(BUILD_DIR)/test_fks_static
SHARED_BIN = $(BUILD_DIR)/test_fks_shared

$(BUILD_DIR)/%.o-static: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o-shared: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -fPIC -o $@ $<

$(STATIC_LIB): $(STATIC_OBJS)	
	mkdir -p $(dir $@)
	ar rcs $@ $^

$(SHARED_LIB): $(SHARED_OBJS)
	mkdir -p $(dir $@)
	$(CC) -shared -o $@ $^

test_fks: test_fks.c $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) -o $(BIN) $(CFLAGS) $^  

test_fks_static: test_fks.c $(STATIC_LIB)
	$(CC) -o $(STATIC_BIN) $(CFLAGS) -static $^

test_fks_shared: test_fks.c $(SHARED_LIB)
	$(CC) -o $(SHARED_BIN) $(CFLAGS) -L$(BUILD_DIR) -lhash $^

clean: 
	-rm -rf $(BUILD_DIR)

.PHONY: test_fks clean