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
  state->paused = false;
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
  if (!state || !state->universe)
    return;

  if (IsKeyPressed(KEY_SPACE))
    state->paused = !state->paused;
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
      UniverseSetBoundary(
          state->universe,
          (UniverseBoundary){
              .left = 0.0 + BOUNDARY_PADDING,
              .right = (double)currentWidth - BOUNDARY_PADDING,
              .top = 0.0 + BOUNDARY_PADDING,
              .bottom = (double)currentHeight - BOUNDARY_PADDING,
              .thickness = 1.0,
              .enabled = true,
          });

      // Update cached dimensions
      lastWidth = currentWidth;
      lastHeight = currentHeight;
    }

    float deltaTime = 8 * GetFrameTime();

    if (!state->paused)
      UniverseUpdate(state->universe, deltaTime);
  }
}

void kurage_render(void) {
  if (!state || !state->universe)
    return;

  RenderUniverse(state->universe);
  // RenderUniverseGrid(state->universe);

  if (state->paused) {
    const char *label = "PAUSADO";
    int fontSize = 36;
    int textWidth = MeasureText(label, fontSize);
    int x = (GetScreenWidth() - textWidth) / 2;
    int y = GetScreenHeight() / 2 - fontSize / 2;
    DrawText(label, x, y, fontSize, YELLOW);
  }
}

void kurage_shutdown(void) {
  if (!state) {
    return;
  }
  if (state->universe) {
    UniverseDestroy(state->universe);
    state->universe = NULL;
  }
  free(state);
  state = NULL;
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
    UniverseSetBoundary(state->universe,
                        (UniverseBoundary){
                            .left = 0.0 + BOUNDARY_PADDING,
                            .right = (double)windowWidth - BOUNDARY_PADDING,
                            .top = 0.0 + BOUNDARY_PADDING,
                            .bottom = (double)windowHeight - BOUNDARY_PADDING,
                            .thickness = 1.0,
                            .enabled = true,
                        });

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

      double radius = 15.0; // + ((double)rand() / (double)RAND_MAX) * 10.0;
      double density = 1.0;
      double mass = M_PI * radius * radius * density;
      double vel_x = -10.0 + ((double)rand() / (double)RAND_MAX) * 20.0;
      double vel_y = -10.0 + ((double)rand() / (double)RAND_MAX) * 20.0;
      KVector2 velocity = {vel_x, vel_y};
      KVector2 position = {x, y};
      EntityID entity = UniverseCreateEntity(state->universe);
      if (entity != INVALID_ENTITY) {
        UniverseAddKParticle(state->universe, entity,
                             (KParticle){
                                 .radius = radius,
                             });
        UniverseAddKMechanic(state->universe, entity,
                             (KMechanic){
                                 .pos = position,
                                 .prev_pos = position,
                                 .vel = velocity,
                                 .acc = (KVector2){0.0, 0.0},
                             });
        UniverseAddKBody(state->universe, entity,
                         (KBody){
                             .invMass = (mass > 0.0) ? 1.0 / mass : 0.0,
                             .mass = mass,
                         });
      }
    }
  }
}