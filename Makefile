##############################################
# Kurage Project Makefile
##############################################

# Minimal, clear Makefile for Kurage
# Targets: all, build, reload, test, valgrind-test, cppcheck, run, clean

# Raylib
LIB_DIR := lib
RAYLIB_DIR := $(LIB_DIR)/raylib
RAYLIB_SRC := $(RAYLIB_DIR)/src
RAYLIB_LIB := $(RAYLIB_SRC)/libraylib.so

# Paths
BUILD_DIR := build
BIN := $(BUILD_DIR)/kurage
ENGINE_SO := $(BUILD_DIR)/libkurage.so
TEST_BIN := $(BUILD_DIR)/leak_test
BIN_DIR := $(BUILD_DIR)/bin

# Tools
CC := gcc
CFLAGS := -Wall -Wl,-rpath=./$(RAYLIB_SRC)
LDFLAGS := -L./$(RAYLIB_SRC) -lraylib -lm -lpthread -ldl -lrt -lX11
INCLUDES := -I./$(RAYLIB_SRC) -Isrc
SHARED_FLAGS := -shared -fPIC

# Output files
KURAGE_BIN = $(BUILD_DIR)/kurage
ENGINE_SO = $(BUILD_DIR)/libkurage.so

# Source files
MAIN_SRC = src/main.c
ENGINE_SRC = src/engine/engine.c src/engine/kurage_math.c
KURAGE_SRC = src/engine/kurage.c
TEST_SRC = tests/leak_test.c
VERLET_TEST_SRC = tests/verlet_test.c
VERLET_TEST_BIN = $(BUILD_DIR)/verlet_test

.PHONY: all build reload test valgrind-test cppcheck check run clean dirs

# Default target
all: build

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(BIN_DIR)

# Build everything (binary + shared lib)
build: $(BIN) reload
	@echo "build: done"

# Build main executable
$(KURAGE_BIN): $(MAIN_SRC) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) $(MAIN_SRC) -o $(KURAGE_BIN) $(LDFLAGS)

# Build shared engine (hot-reloadable)
reload: $(ENGINE_SRC) $(KURAGE_SRC) $(RAYLIB_LIB)
	$(CC) -shared -fPIC $(CFLAGS) $(INCLUDES) $(ENGINE_SRC) $(KURAGE_SRC) -o $(ENGINE_SO) $(LDFLAGS)

# ----------------------
# Tests & checks
# ----------------------
test: $(TEST_BIN) $(VERLET_TEST_BIN)
	@echo "Running leak_test..."
	@$(TEST_BIN)
	@echo "Running verlet_test..."
	@$(VERLET_TEST_BIN)

$(TEST_BIN): $(TEST_SRC) | $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_BIN)
	@echo "Built $(TEST_BIN)"

$(VERLET_TEST_BIN): $(VERLET_TEST_SRC) $(ENGINE_SRC) | $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) $(VERLET_TEST_SRC) $(ENGINE_SRC) -o $(VERLET_TEST_BIN) -lm
	@echo "Built $(VERLET_TEST_BIN)"

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
# Raylib
# ----------------------
$(RAYLIB_LIB): dirs
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
run: all
	@echo "Running $(KURAGE_BIN)"
	@$(KURAGE_BIN)

clean:
	rm -rf $(BUILD_DIR)
	@echo "clean: done"