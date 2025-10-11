#include "../lib/raylib/src/raylib.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations of function types
typedef void (*kurage_init_t)(void);
typedef void *(*kurage_pre_reload_t)(void);
typedef void (*kurage_post_reload_t)(void *);
typedef void (*kurage_logic_t)(void);
typedef void (*kurage_update_t)(void);
typedef void (*kurage_render_t)(void);

// Function pointers
kurage_init_t kurage_init = NULL;
kurage_pre_reload_t kurage_pre_reload = NULL;
kurage_post_reload_t kurage_post_reload = NULL;
kurage_logic_t kurage_logic = NULL;
kurage_update_t kurage_update = NULL;
kurage_render_t kurage_render = NULL;

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

  // Load all the function pointers
  kurage_init = dlsym(engine_lib, "kurage_init");
  if (kurage_init == NULL) {
    fprintf(stderr, "ERROR: could not find kurage_init symbol in %s: %s\n",
            engine_lib_name, dlerror());
    return -1;
  }

  kurage_pre_reload = dlsym(engine_lib, "kurage_pre_reload");
  if (kurage_pre_reload == NULL) {
    fprintf(stderr,
            "ERROR: could not find kurage_pre_reload symbol in %s: %s\n",
            engine_lib_name, dlerror());
    return -1;
  }

  kurage_post_reload = dlsym(engine_lib, "kurage_post_reload");
  if (kurage_post_reload == NULL) {
    fprintf(stderr,
            "ERROR: could not find kurage_post_reload symbol in %s: %s\n",
            engine_lib_name, dlerror());
    return -1;
  }

  kurage_logic = dlsym(engine_lib, "kurage_logic");
  if (kurage_logic == NULL) {
    fprintf(stderr, "ERROR: could not find kurage_logic symbol in %s: %s\n",
            engine_lib_name, dlerror());
    return -1;
  }

  kurage_update = dlsym(engine_lib, "kurage_update");
  if (kurage_update == NULL) {
    fprintf(stderr, "ERROR: could not find kurage_update symbol in %s: %s\n",
            engine_lib_name, dlerror());
    return -1;
  }

  kurage_render = dlsym(engine_lib, "kurage_render");
  if (kurage_render == NULL) {
    fprintf(stderr, "ERROR: could not find kurage_render symbol in %s: %s\n",
            engine_lib_name, dlerror());
    return -1;
  }

  return 1;
}

int main(void) {
  size_t factor = 50;
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
    kurage_render();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}