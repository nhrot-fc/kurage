##############################################
# Kurage Project Makefile
##############################################

# Paths
BUILD_DIR := build
LIB_DIR := lib

# Raylib
RAYLIB_DIR := $(LIB_DIR)/raylib
RAYLIB_SRC := $(RAYLIB_DIR)/src
RAYLIB_LIB := $(RAYLIB_SRC)/libraylib.so

# Tools
CC := gcc
CFLAGS := -Wall -Wl,-rpath=./$(RAYLIB_SRC)
LDFLAGS := -L./$(RAYLIB_SRC) -lraylib -lm -lpthread -ldl -lrt -lX11
INCLUDES := -I./$(RAYLIB_SRC) -Isrc
SHARED_FLAGS := -shared -fPIC

# Source files
MAIN_SRC = src/main.c
ENGINE_SRC = $(wildcard src/core/*.c) $(wildcard src/core/physics/*.c) \
	           $(wildcard src/core/math/*.c)
PLUGIN_SRC = $(wildcard src/plugin/*.c) $(wildcard src/render/*.c)
TEST_SRC = $(wildcard tests/*.c)
TEST_BUILD_DIR := $(BUILD_DIR)/tests
TEST_BINS := $(patsubst tests/%.c,$(TEST_BUILD_DIR)/%,$(TEST_SRC))
TEST_LDFLAGS := -lm

# Output files
KURAGE_BIN = $(BUILD_DIR)/kurage
PLUGIN_SO = $(BUILD_DIR)/plugin.so

.PHONY: all test reload run clean

# Default target
all: run

# Build main executable
$(KURAGE_BIN): $(MAIN_SRC) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) $(MAIN_SRC) -o $(KURAGE_BIN) $(LDFLAGS)

reload: $(ENGINE_SRC) $(PLUGIN_SRC) $(RAYLIB_LIB)
	@mkdir -p $(BUILD_DIR)
	$(CC) -shared -fPIC $(CFLAGS) $(INCLUDES) $(ENGINE_SRC) $(PLUGIN_SRC) -o $(PLUGIN_SO) $(LDFLAGS)
	@echo "reload: done"

# ----------------------
# Raylib
# ----------------------
$(RAYLIB_LIB):
	@mkdir -p $(LIB_DIR)
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR); \
	fi
	@if [ ! -f "$(RAYLIB_LIB)" ]; then \
		echo "Raylib not built, building now..."; \
		cd $(RAYLIB_SRC) && make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED; \
	fi

# ----------------------
# Run / cleanup
# ----------------------
run: reload $(KURAGE_BIN)
	@echo "Running $(KURAGE_BIN)"
	@$(KURAGE_BIN)

clean:
	rm -rf $(BUILD_DIR)
	@echo "clean: done"

# ----------------------
# Tests
# ----------------------
$(TEST_BUILD_DIR)/%: tests/%.c $(ENGINE_SRC)
	@mkdir -p $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(ENGINE_SRC) -o $@ $(TEST_LDFLAGS)

test: $(TEST_BINS)
	@set -e; \
	for t in $^; do \
		echo "Running $$t"; \
		"$$t"; \
	done
