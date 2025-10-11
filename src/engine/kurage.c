/**
 * kurage.c
 *
 * Implementation of the hot reloadable functions for the Kurage Physics Engine.
 *
 * Author: nhrot-fc
 */

#include "kurage.h"
#include "../../lib/raylib/src/raylib.h"
#include "../config/config.h"
#include <stdio.h>
#include <stdlib.h>

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

  // Here you would render all particles
  // For example:
  for (uint32_t i = 0; i < state->universe->maxEntities; i++) {
    if (state->universe->activeEntities[i] &&
        (state->universe->entityMasks[i] & COMPONENT_PARTICLE)) {

      ParticleComponent *particle = &state->universe->particles[i];

      // Draw particle (using Raylib)
      DrawCircle((int)particle->position.x, (int)particle->position.y,
                 OBJECT_RADIUS, WHITE);
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

    // Create some example particles
    KVector2 pos = {400, 300};
    KVector2 vel = {0, 0};
    ParticleCreate(state->universe, pos, vel, 1.0);
  }
}