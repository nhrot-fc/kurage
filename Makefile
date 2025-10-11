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

all: $(BUILD_DIR) $(KURAGE_BIN) $(ENGINE_SO)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)

# Build main executable
$(KURAGE_BIN): $(MAIN_SRC) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) $(MAIN_SRC) -o $(KURAGE_BIN) $(LDFLAGS)

# Build hot reloadable engine library
$(ENGINE_SO): $(ENGINE_SRC) $(KURAGE_SRC) $(RAYLIB_LIB)
	$(CC) -shared -fPIC $(CFLAGS) $(INCLUDES) $(ENGINE_SRC) $(KURAGE_SRC) -o $(ENGINE_SO) $(LDFLAGS)

# Shortcut to rebuild just the shared library for hot reloading
reload: $(ENGINE_SO)

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