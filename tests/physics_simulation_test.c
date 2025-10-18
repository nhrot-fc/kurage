#include "../src/config/config.h"
#include "../src/core/engine.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include "test_framework.h"

#define EPSILON 0.01
#define DELTA_TIME 0.1

int test_particle_drop_with_gravity(void) {
  // PRECONDICIONES: Universo sin límites con partícula en reposo y gravedad
  // tomada de la configuración global.
  // PROCESO: Aplicar fuerza constante igual a la gravedad y avanzar cinco
  // pasos de integración Verlet.
  // POSTCONDICIONES: La velocidad final debe coincidir con la solución
  // analítica v = g * t dentro del umbral EPSILON.

  KVector2 gravity = {GRAVITY_X, GRAVITY_Y};
  Universe *universe = UniverseCreate(10);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return 1;
  }

  universe->boundary.enabled = false;

  KVector2 pos = {0.0, 0.0};
  KVector2 vel = {0.0, 0.0};
  EntityID particle = UniverseCreateEntity(universe);
  if (particle != INVALID_ENTITY) {
    UniverseAddParticleComponent(universe, particle, OBJECT_RADIUS, 1.0);
    UniverseAddKineticBodyComponent(universe, particle, pos, 1.0);
    UniverseAddMechanicsComponent(universe, particle, vel);
  } else {
    fprintf(stderr, "Failed to create particle\n");
    UniverseDestroy(universe);
    return 1;
  }

  DEBUG_PRINTF("Testing particle drop with gravity...\n");
  DEBUG_PRINTF("Initial position: (%.3f, %.3f)\n", pos.x, pos.y);

  if (!PhysicsApplyConstantForce(universe, particle, gravity)) {
    fprintf(stderr, "Failed to register constant gravity force\n");
    UniverseDestroy(universe);
    return 1;
  }

  for (int step = 0; step < 5; ++step) {
    UniverseUpdate(universe, DELTA_TIME);

#ifdef TEST_DEBUG
    KineticBodyComponent *body_debug =
        UniverseGetKineticBodyComponent(universe, particle);
    MechanicsComponent *mech_debug =
        UniverseGetMechanicsComponent(universe, particle);

    DEBUG_PRINTF("Step %d: pos=(%.3f, %.3f), vel=(%.3f, %.3f)\n", step + 1,
                 body_debug->position.x, body_debug->position.y,
                 mech_debug->velocity.x, mech_debug->velocity.y);
#endif
  }

  MechanicsComponent *mech = UniverseGetMechanicsComponent(universe, particle);
  double expected_vel = KVector2Norm(gravity) * (5 * DELTA_TIME);

#ifdef TEST_DEBUG
  KineticBodyComponent *body =
      UniverseGetKineticBodyComponent(universe, particle);

  DEBUG_PRINTF("\nExpected velocity: %.3f\n", expected_vel);
  DEBUG_PRINTF("Actual velocity: %.3f\n", mech->velocity.y);
  DEBUG_PRINTF(
      "Position: %.3f (has initial offset due to Verlet characteristics)\n",
      body->position.y);
#endif

  double velocity_delta = fabs(mech->velocity.y - expected_vel);
  if (velocity_delta >= EPSILON) {
    fprintf(stderr,
            "Velocity mismatch: expected %.3f, got %.3f (|delta|=%.3f)\n",
            expected_vel, mech->velocity.y, velocity_delta);
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_particle_drop_with_gravity", test_particle_drop_with_gravity},
  };

  return RUN_TEST_SUITE("PHYSICS", tests);
}
