#ifndef KURAGE_H
#define KURAGE_H

#include <stdbool.h>

#include "../core/engine.h"

/**
 * State structure to hold the engine's state for hot reloading
 * This allows preserving the simulation state between reloads
 */
typedef struct {
  Universe *universe;
  bool paused;
  // Add any other state variables that need to be preserved
} KurageState;

/**
 * Function list for hot reloadable functions
 * This list defines the interface between the main application and the
 * hot-reloadable library
 */
#define KURAGE_FUNC_LIST                                                       \
  X(kurage_init, void, void)                                                   \
  X(kurage_pre_reload, struct KurageState *, void)                             \
  X(kurage_post_reload, void, struct KurageState *)                            \
  X(kurage_logic, void, void)                                                  \
  X(kurage_update, void, void)                                                 \
  X(kurage_render, void, void)                                                 \
  X(kurage_shutdown, void, void)

#endif // KURAGE_H