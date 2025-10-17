/**
 * kurage.c
 *
 * Implementation of the hot reloadable functions for the Kurage Physics Engine.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../lib/raylib/src/raylib.h"
#include "../config/config.h"
#include "../render/draw.h"
#include "plugin.h"

// Global state to be preserved between reloads
static KurageState *state = NULL;

// Static function declarations
static void init_universe(void);

void kurage_init(void) {
  printf("Initializing Kurage Physics Engine\n");

  // Create our state structure
  state = (KurageState *)malloc(sizeof(KurageState));
  if (!state) {
    fprintf(stderr, "ERROR: Failed to allocate state memory\n");
    return;
  }

  // Initialize universe
  init_universe();
}

KurageState *kurage_pre_reload(void) {
  printf("Preparing for hot reload...\n");
  return state;
}

void kurage_post_reload(KurageState *preserved_state) {
  printf("Restoring state after hot reload...\n");
  state = preserved_state;
}

void kurage_logic(void) {
  // Handle any physics engine logic here
  // For example, detecting user input for physics objects
}

void kurage_update(void) {
  // Update physics simulation
  if (state && state->universe) {
    // Check if window has been resized and update boundaries
    static int lastWidth = 0;
    static int lastHeight = 0;
    int currentWidth = GetScreenWidth();
    int currentHeight = GetScreenHeight();

    if (currentWidth != lastWidth || currentHeight != lastHeight) {
      // Window resized, update boundaries
      UniverseSetBoundaries(state->universe, currentWidth, currentHeight,
                            BOUNDARY_PADDING, true);

      // Update cached dimensions
      lastWidth = currentWidth;
      lastHeight = currentHeight;
    }

    float deltaTime = 8 * GetFrameTime();

    UniverseUpdate(state->universe, deltaTime);
  }
}

void kurage_render(void) {
  if (!state || !state->universe)
    return;

  RenderUniverse(state->universe);
}

// Initialize the physics universe
static void init_universe(void) {
  if (state) {
    state->universe = UniverseCreate(MAX_OBJECTS);
    if (!state->universe) {
      fprintf(stderr, "ERROR: Failed to create universe\n");
      return;
    }

    // Set universe boundaries based on window size with padding
    int windowWidth = GetScreenWidth();
    int windowHeight = GetScreenHeight();
    UniverseSetBoundaries(state->universe, windowWidth, windowHeight,
                          BOUNDARY_PADDING, true);

    srand(time(NULL));
    const double left = state->universe->boundary.left;
    const double right = state->universe->boundary.right;
    const double top = state->universe->boundary.top;
    const double bottom = state->universe->boundary.bottom;
    const double width = right - left;
    const double height = bottom - top;

    for (uint32_t i = 0; i < state->universe->maxEntities; i++) {
      double x = left;
      double y = top;

      if (width > 0.0) {
        x += ((double)rand() / (double)RAND_MAX) * width;
      }
      if (height > 0.0) {
        y += ((double)rand() / (double)RAND_MAX) * height;
      }

      double velx = rand() % 80 - 40;
      double vely = 0; // rand() % 80 - 40;
      double mass = (rand() % 100 + 1) / 100.0;
      ParticleCreate(state->universe, (KVector2){x, y}, (KVector2){velx, vely},
                     mass);
    }
  }
}