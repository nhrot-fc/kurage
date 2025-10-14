#include "../lib/raylib/src/raylib.h"
#include "engine/kurage.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define function pointer types
#define X(name, ret, ...) typedef ret (*name##_t)(__VA_ARGS__);
KURAGE_FUNC_LIST
#undef X

// Declare function pointers
#define X(name, ...) name##_t name = NULL;
KURAGE_FUNC_LIST
#undef X

const char *engine_lib_name = "build/libkurage.so";
void *engine_lib = NULL;

int lib_reload(void) {
  if (engine_lib != NULL)
    dlclose(engine_lib);

  engine_lib = dlopen(engine_lib_name, RTLD_NOW);

  if (engine_lib == NULL) {
    fprintf(stderr, "ERROR: could not load %s: %s\n", engine_lib_name,
            dlerror());
    return -1;
  }

// Load all function pointers using the macro
#define X(name, ...)                                                           \
  name = (name##_t)dlsym(engine_lib, #name);                                   \
  if (name == NULL) {                                                          \
    fprintf(stderr, "ERROR: could not find %s symbol in %s: %s\n", #name,      \
            engine_lib_name, dlerror());                                       \
    return -1;                                                                 \
  }
  KURAGE_FUNC_LIST
#undef X

  return 1;
}

int main(void) {
  size_t factor = 100;
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(factor * 16, factor * 9, "Kurage Physics Engine");
  SetTargetFPS(60);

  // Load the engine library
  if (lib_reload() == -1) {
    CloseWindow();
    return 1;
  }

  // Initialize the engine
  kurage_init();

  while (!WindowShouldClose()) {
    // Check for reloading library
    if (IsKeyPressed(KEY_ESCAPE))
      break;
    if (IsKeyPressed(KEY_R)) {
      void *state = kurage_pre_reload();
      if (lib_reload() == -1)
        return 1;
      kurage_post_reload(state);
    }

    // Update and render
    kurage_logic();
    kurage_update();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(0, 0);
    kurage_render();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}