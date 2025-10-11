#ifndef KURAGE_H
#define KURAGE_H

#include "engine.h"

/**
 * State structure to hold the engine's state for hot reloading
 * This allows preserving the simulation state between reloads
 */
typedef struct {
  Universe *universe;
  // Add any other state variables that need to be preserved
} KurageState;

/**
 * Function list for hot reloadable functions
 * This list defines the interface between the main application and the
 * hot-reloadable library
 */
#define KURAGE_FUNC_LIST                                                       \
  X(kurage_init, void, void)                                                   \
  X(kurage_pre_reload, KurageState *, void)                                    \
  X(kurage_post_reload, void, KurageState *)                                   \
  X(kurage_logic, void, void)                                                  \
  X(kurage_update, void, void)                                                 \
  X(kurage_render, void, void)

// When used in the implementation file, declare the functions
#ifdef KURAGE_IMPLEMENTATION
void kurage_init(void);
KurageState *kurage_pre_reload(void);
void kurage_post_reload(KurageState *state);
void kurage_logic(void);
void kurage_update(void);
void kurage_render(void);
#endif

#endif // KURAGE_H