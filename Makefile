##############################################
# Kurage Project Makefile
##############################################

# Directories
SRC_DIR = src
LIB_DIR = lib
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj

# Output files
ENGINE_LIB = $(BUILD_DIR)/libengine.a
TARGET = $(BIN_DIR)/kurage

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
PLUGIN_CFLAGS = $(CFLAGS) -fPIC

# Raylib
RAYLIB_DIR = $(LIB_DIR)/raylib
RAYLIB_SRC = $(RAYLIB_DIR)/src
RAYLIB_LIB = $(RAYLIB_SRC)/libraylib.a

# Include path, Libraries and linker flags
INCLUDES = -I$(SRC_DIR) -I$(RAYLIB_SRC)
LDFLAGS = -L$(BUILD_DIR) -L$(RAYLIB_SRC)
LDLIBS = -lengine -lraylib -lm -lpthread -ldl -lrt -lX11

# Source files
ENGINE_SRC = $(SRC_DIR)/engine/engine.c
CONFIG_SRC = 
MAIN_SRC = $(SRC_DIR)/main.c

# Object files
ENGINE_OBJ = $(OBJ_DIR)/engine.o
MAIN_OBJ = $(OBJ_DIR)/main.o

# Default target
all: dirs compile-engine $(TARGET)

# Create necessary directories
dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR) $(OBJ_DIR) $(OBJ_DIR)/engine

compile-engine: dirs
	@echo "Compiling engine sources..."
	@mkdir -p $(dir $(ENGINE_OBJ))
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(ENGINE_SRC) -o $(ENGINE_OBJ)
	@echo "Building engine library..."
	@ar rcs $(ENGINE_LIB) $(ENGINE_OBJ)

# Setup Raylib
setup:
	@echo "Setting up Raylib..."
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR); \
	else \
		echo "Raylib already exists. Skipping clone."; \
	fi
	@cd $(RAYLIB_SRC) && make PLATFORM=PLATFORM_DESKTOP

# Compile main.c file
$(MAIN_OBJ): $(MAIN_SRC)
	@echo "Compiling main source..."
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link the final executable
$(TARGET): $(MAIN_OBJ) $(ENGINE_LIB) $(RAYLIB_LIB)
	@echo "Building main application..."
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) $(LDFLAGS) $(LDLIBS)

# Run the application
run: all
	@echo "Running Kurage..."
	@$(TARGET)

# Clean build files
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)

.PHONY: all clean compile-engine run setup dirs
