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
#include "engine.h"
#include "kurage.h"
#include "kurage_math.h"

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
      float padding = 10.0f;
      UniverseSetBoundaries(state->universe, currentWidth, currentHeight,
                            padding, true);

      // Update cached dimensions
      lastWidth = currentWidth;
      lastHeight = currentHeight;
    }

    // Apply forces
    KVector2 gravity = {0, GRAVITY};
    PhysicsApplyGravity(state->universe, gravity);

    // Integrate forces and update positions
    PhysicsIntegrateForces(state->universe, DELTA_TIME);
  }
}

void kurage_render(void) {
  // Render physics objects
  if (!state || !state->universe)
    return;

  // Draw universe boundaries if they're enabled
  if (state->universe->boundary.enabled) {
    Color boundaryColor = ColorAlpha(WHITE, 0.8f); // Semitransparent white

    // Draw rectangle outline for the boundary
    DrawRectangleLines(
        (int)state->universe->boundary.left, (int)state->universe->boundary.top,
        (int)(state->universe->boundary.right - state->universe->boundary.left),
        (int)(state->universe->boundary.bottom - state->universe->boundary.top),
        boundaryColor);
  }

  // Render all particles
  for (uint32_t i = 0; i < state->universe->maxEntities; i++) {
    if (state->universe->activeEntities[i] &&
        (state->universe->entityMasks[i] & COMPONENT_PARTICLE)) {

      KineticBodyComponent *particle = &state->universe->kineticBodies[i];

      // Get velocity for coloring (if mechanics component exists)
      Color particleColor = WHITE;
      if (state->universe->entityMasks[i] & COMPONENT_MECHANICS) {
        MechanicsComponent *mechanics = &state->universe->mechanics[i];
        double speed = sqrt(mechanics->velocity.x * mechanics->velocity.x +
                            mechanics->velocity.y * mechanics->velocity.y);

        // Color based on speed (blue=slow, white=medium, red=fast)
        if (speed > 100) {
          particleColor = RED;
        } else if (speed > 50) {
          particleColor = ORANGE;
        } else if (speed > 20) {
          particleColor = YELLOW;
        } else if (speed > 10) {
          particleColor = GREEN;
        } else {
          particleColor = BLUE;
        }
      }

      // Draw particle
      DrawCircle((int)particle->position.x, (int)particle->position.y,
                 OBJECT_RADIUS, particleColor);
    }
  }
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
    float padding = 10.0f; // 10 pixels padding from screen edge
    UniverseSetBoundaries(state->universe, windowWidth, windowHeight, padding,
                          true);

    srand(time(NULL));
    for (int i = 0; i < KURAGE_MAX_ENTITIES; i++) {
      double x = rand() % (int)state->universe->boundary.right +
                 state->universe->boundary.left;
      double y = rand() % (int)state->universe->boundary.bottom +
                 state->universe->boundary.top;

      double velx = rand() % 80 - 40;
      double vely = rand() % 80 - 40;
      double mass = (rand() % 100 + 1) / 100.0;
      ParticleCreate(state->universe, (KVector2){x, y}, (KVector2){velx, vely},
                     mass);
    }
  }
}