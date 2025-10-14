##############################################
# Kurage Project Makefile
##############################################

# Variables
CC = gcc
CFLAGS = -Wall -Wl,-rpath=./lib/raylib/src
LDFLAGS = -L./lib/raylib/src -lraylib -lm -lpthread -ldl -lrt -lX11
INCLUDES = -I./lib/raylib/src -Isrc
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin

# Output files
KURAGE_BIN = $(BUILD_DIR)/kurage
ENGINE_SO = $(BUILD_DIR)/libkurage.so

# Source files
MAIN_SRC = src/main.c
ENGINE_SRC = src/engine/engine.c src/engine/kurage_math.c
KURAGE_SRC = src/engine/kurage.c

# Raylib
RAYLIB_LIB = ./lib/raylib/src/libraylib.a

all: $(BUILD_DIR) $(KURAGE_BIN) reload

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)

# Build main executable
$(KURAGE_BIN): $(MAIN_SRC) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) $(MAIN_SRC) -o $(KURAGE_BIN) $(LDFLAGS)

# Build hot reloadable engine library
reload: $(ENGINE_SRC) $(KURAGE_SRC) $(RAYLIB_LIB)
	$(CC) -shared -fPIC $(CFLAGS) $(INCLUDES) $(ENGINE_SRC) $(KURAGE_SRC) -o $(ENGINE_SO) $(LDFLAGS)

# --------------------
# Tests & checks
# --------------------
TEST_BIN = $(BUILD_DIR)/leak_test
TEST_SRC = tests/leak_test.c

.PHONY: test valgrind-test cppcheck check

test: $(TEST_BIN)
	@echo "Running leak_test (simple runtime smoke test)..."
	@$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SRC) -o $(TEST_BIN)

valgrind-test: $(TEST_BIN)
	@echo "Running leak_test under Valgrind..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --error-exitcode=2 ./$(TEST_BIN); \
	else \
		echo "Valgrind not found; install valgrind to run this check"; exit 0; \
	fi

cppcheck:
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=warning,performance,portability --inconclusive --std=c11 --force src || true; \
	else \
		echo "cppcheck not found; skipping static analysis"; \
	fi

check: cppcheck test
	@echo "Checks completed. Try 'make valgrind-test' locally for leak detection."


# Build Raylib if needed
$(RAYLIB_LIB):
	@echo "Building Raylib..."
	@cd ./lib/raylib/src && make PLATFORM=PLATFORM_DESKTOP

# Run the application
run: all
	@echo "Running Kurage..."
	@$(KURAGE_BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean reload run