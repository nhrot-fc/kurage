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

# Output files
KURAGE_BIN = $(BUILD_DIR)/kurage
PLUGIN_SO = $(BUILD_DIR)/plugin.so

# Test files
TEST_SRC = tests/leak_test.c
TEST_BIN = $(BUILD_DIR)/leak_test
VERLET_TEST_SRC = tests/verlet_test.c
VERLET_TEST_BIN = $(BUILD_DIR)/verlet_test
PHYSICS_SIM_TEST_SRC = tests/physics_simulation_test.c
PHYSICS_SIM_TEST_BIN = $(BUILD_DIR)/physics_sim_test
PHYSICS_FORCE_TEST_SRC = tests/physics_force_test.c
PHYSICS_FORCE_TEST_BIN = $(BUILD_DIR)/physics_force_test
PHYSICS_MECHANICS_TEST_SRC = tests/physics_mechanics_test.c
PHYSICS_MECHANICS_TEST_BIN = $(BUILD_DIR)/physics_mechanics_test
PHYSICS_COLLISION_TEST_SRC = tests/physics_collision_test.c
PHYSICS_COLLISION_TEST_BIN = $(BUILD_DIR)/physics_collision_test
PHYSICS_BOUNDARY_TEST_SRC = tests/physics_boundary_test.c
PHYSICS_BOUNDARY_TEST_BIN = $(BUILD_DIR)/physics_boundary_test

TEST_DEFINES ?=

.PHONY: all build reload test valgrind-test cppcheck check run clean dirs

# Default target
all: build

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)

# Build everything (binary + shared lib)
build: $(KURAGE_BIN) reload
	@echo "build: done"

# Build main executable
$(KURAGE_BIN): $(MAIN_SRC) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) $(MAIN_SRC) -o $(KURAGE_BIN) $(LDFLAGS)

reload: $(ENGINE_SRC) $(PLUGIN_SRC) $(RAYLIB_LIB)
	$(CC) -shared -fPIC $(CFLAGS) $(INCLUDES) $(ENGINE_SRC) $(PLUGIN_SRC) -o $(PLUGIN_SO) $(LDFLAGS)

# ----------------------
# Tests & checks
# ----------------------
test: $(TEST_BIN) $(VERLET_TEST_BIN) $(PHYSICS_SIM_TEST_BIN) \
      $(PHYSICS_FORCE_TEST_BIN) $(PHYSICS_MECHANICS_TEST_BIN) \
      $(PHYSICS_COLLISION_TEST_BIN) $(PHYSICS_BOUNDARY_TEST_BIN)
	@echo "Running leak_test..."
	@$(TEST_BIN)
	@echo "Running verlet_test..."
	@$(VERLET_TEST_BIN)
	@echo "Running physics_sim_test..."
	@$(PHYSICS_SIM_TEST_BIN)
	@echo "Running physics_force_test..."
	@$(PHYSICS_FORCE_TEST_BIN)
	@echo "Running physics_mechanics_test..."
	@$(PHYSICS_MECHANICS_TEST_BIN)
	@echo "Running physics_collision_test..."
	@$(PHYSICS_COLLISION_TEST_BIN)
	@echo "Running physics_boundary_test..."
	@$(PHYSICS_BOUNDARY_TEST_BIN)

test-debug: TEST_DEFINES += -DTEST_DEBUG
test-debug: test


$(TEST_BIN): $(TEST_SRC) | dirs
	$(CC) $(CFLAGS) $(TEST_DEFINES) $(INCLUDES) $(TEST_SRC) -o $(TEST_BIN) -lm
	@echo "Built $(TEST_BIN)"

$(VERLET_TEST_BIN): $(VERLET_TEST_SRC) $(ENGINE_SRC) | $(RAYLIB_LIB) dirs
	$(CC) $(CFLAGS) $(TEST_DEFINES) $(INCLUDES) $(VERLET_TEST_SRC) $(ENGINE_SRC) -o $(VERLET_TEST_BIN) -lm
	@echo "Built $(VERLET_TEST_BIN)"

$(PHYSICS_SIM_TEST_BIN): $(PHYSICS_SIM_TEST_SRC) $(ENGINE_SRC) | $(RAYLIB_LIB) dirs
	$(CC) $(CFLAGS) $(TEST_DEFINES) $(INCLUDES) $(PHYSICS_SIM_TEST_SRC) $(ENGINE_SRC) -o $(PHYSICS_SIM_TEST_BIN) -lm
	@echo "Built $(PHYSICS_SIM_TEST_BIN)"

$(PHYSICS_FORCE_TEST_BIN): $(PHYSICS_FORCE_TEST_SRC) $(ENGINE_SRC) | $(RAYLIB_LIB) dirs
	$(CC) $(CFLAGS) $(TEST_DEFINES) $(INCLUDES) $(PHYSICS_FORCE_TEST_SRC) $(ENGINE_SRC) -o $(PHYSICS_FORCE_TEST_BIN) -lm
	@echo "Built $(PHYSICS_FORCE_TEST_BIN)"

$(PHYSICS_MECHANICS_TEST_BIN): $(PHYSICS_MECHANICS_TEST_SRC) $(ENGINE_SRC) | $(RAYLIB_LIB) dirs
	$(CC) $(CFLAGS) $(TEST_DEFINES) $(INCLUDES) $(PHYSICS_MECHANICS_TEST_SRC) $(ENGINE_SRC) -o $(PHYSICS_MECHANICS_TEST_BIN) -lm
	@echo "Built $(PHYSICS_MECHANICS_TEST_BIN)"

$(PHYSICS_COLLISION_TEST_BIN): $(PHYSICS_COLLISION_TEST_SRC) $(ENGINE_SRC) | $(RAYLIB_LIB) dirs
	$(CC) $(CFLAGS) $(TEST_DEFINES) $(INCLUDES) $(PHYSICS_COLLISION_TEST_SRC) $(ENGINE_SRC) -o $(PHYSICS_COLLISION_TEST_BIN) -lm
	@echo "Built $(PHYSICS_COLLISION_TEST_BIN)"

$(PHYSICS_BOUNDARY_TEST_BIN): $(PHYSICS_BOUNDARY_TEST_SRC) $(ENGINE_SRC) | $(RAYLIB_LIB) dirs
	$(CC) $(CFLAGS) $(TEST_DEFINES) $(INCLUDES) $(PHYSICS_BOUNDARY_TEST_SRC) $(ENGINE_SRC) -o $(PHYSICS_BOUNDARY_TEST_BIN) -lm
	@echo "Built $(PHYSICS_BOUNDARY_TEST_BIN)"

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