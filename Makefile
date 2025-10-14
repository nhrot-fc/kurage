##############################################
# Kurage Project Makefile
##############################################

# Minimal, clear Makefile for Kurage
# Targets: all, build, reload, test, valgrind-test, cppcheck, run, clean

# Tools
CC := gcc

# Flags
CFLAGS := -Wall -Wextra -I./lib/raylib/src -Isrc
LDFLAGS := -L./lib/raylib/src -lraylib -lm -lpthread -ldl -lrt -lX11
SHARED_FLAGS := -shared -fPIC

# Paths
BUILD_DIR := build
BIN := $(BUILD_DIR)/kurage
ENGINE_SO := $(BUILD_DIR)/libkurage.so
TEST_BIN := $(BUILD_DIR)/leak_test

# Sources
MAIN_SRC := src/main.c
ENGINE_SRC := src/engine/engine.c src/engine/kurage_math.c
KURAGE_SRC := src/engine/kurage.c
TEST_SRC := tests/leak_test.c

# Raylib
RAYLIB_DIR := lib/raylib
RAYLIB_SRC := $(RAYLIB_DIR)/src
RAYLIB_LIB := $(RAYLIB_SRC)/libraylib.a

.PHONY: all build reload test valgrind-test cppcheck check run clean setup-raylib

# Default target
all: build

# Build everything (binary + shared lib)
build: $(BIN) reload
	@echo "build: done"

# Ensure build directories exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build main executable
$(BIN): $(MAIN_SRC) | $(BUILD_DIR) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(MAIN_SRC) -o $(BIN) $(LDFLAGS)
	@echo "Built $(BIN)"

# Build shared engine (hot-reloadable)
reload: | $(BUILD_DIR) $(RAYLIB_LIB)
	$(CC) $(SHARED_FLAGS) $(CFLAGS) $(ENGINE_SRC) $(KURAGE_SRC) -o $(ENGINE_SO) $(LDFLAGS)
	@echo "Built $(ENGINE_SO)"

# ----------------------
# Tests & checks
# ----------------------
test: $(TEST_BIN)
	@echo "Running leak_test..."
	@$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_BIN)
	@echo "Built $(TEST_BIN)"

valgrind-test: $(TEST_BIN)
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --error-exitcode=2 ./$(TEST_BIN); \
	else \
		echo "valgrind not installed; skipping valgrind-test"; \
	fi

cppcheck:
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=warning,performance,portability --inconclusive --std=c11 --force src || true; \
	else \
		echo "cppcheck not installed; skipping static analysis"; \
	fi

check: cppcheck test
	@echo "All quick checks passed (except optional valgrind)."

# ----------------------
# Raylib helper
# ----------------------
setup-raylib:
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "Cloning raylib..."; git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR); \
	fi
	@echo "Building raylib...";
	@cd $(RAYLIB_SRC) && make PLATFORM=PLATFORM_DESKTOP -j$(shell nproc)

$(RAYLIB_LIB): setup-raylib
	@echo "Raylib built -> $(RAYLIB_LIB)"

# ----------------------
# Run / cleanup
# ----------------------
run: all
	@echo "Running $(BIN)"
	@$(BIN)

clean:
	rm -rf $(BUILD_DIR)
	@echo "clean: done"