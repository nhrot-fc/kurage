#include "../src/core/engine.h"
#include <math.h>
#include <stdio.h>

#include "test_framework.h"
#define EPSILON 0.001
#define DELTA_TIME 0.1

int test_verlet_integration() {
  // PRECONDICIONES: Universo inicializado sin límites y partícula con
  // posición/velocidad inicial definida.
  // PROCESO: Ejecutar una integración de Verlet sin fuerzas para validar la
  // conservación de velocidad y desplazamiento esperado.
  // POSTCONDICIONES: Posición y velocidad finales deben coincidir con la
  // solución analítica dentro de EPSILON.

  // Create a universe
  Universe *universe = UniverseCreate(100);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return 1;
  }
  universe->boundary.enabled = false;

  KVector2 initial_pos = {100.0, 100.0};
  KVector2 initial_vel = {10.0, 0.0};
  double mass = 1.0;

  EntityID particle = UniverseCreateEntity(universe);
  if (particle != INVALID_ENTITY) {
    UniverseAddParticleComponent(universe, particle, OBJECT_RADIUS, 1.0);
    UniverseAddKineticBodyComponent(universe, particle, initial_pos, mass);
    UniverseAddMechanicsComponent(universe, particle, initial_vel);
  } else {
    fprintf(stderr, "Failed to create particle\n");
    UniverseDestroy(universe);
    return 1;
  }

  KineticBodyComponent *body =
      UniverseGetKineticBodyComponent(universe, particle);
  MechanicsComponent *mech = UniverseGetMechanicsComponent(universe, particle);

  DEBUG_PRINTF("Initial position: (%.2f, %.2f)\n", body->position.x,
               body->position.y);
  DEBUG_PRINTF("Initial velocity: (%.2f, %.2f)\n", mech->velocity.x,
               mech->velocity.y);

  UniverseUpdate(universe, DELTA_TIME);
  double expected_x = initial_pos.x + initial_vel.x * DELTA_TIME;
  double expected_y = initial_pos.y + initial_vel.y * DELTA_TIME;
  DEBUG_PRINTF("Position: (%.2f, %.2f)\n", body->position.x, body->position.y);
  DEBUG_PRINTF("Expected Position: (%.2f, %.2f)\n", expected_x, expected_y);

  if (fabs(body->position.x - expected_x) > EPSILON ||
      fabs(body->position.y - expected_y) > EPSILON) {
    fprintf(stderr,
            "Position mismatch: expected (%.2f, %.2f), got (%.2f, %.2f)\n",
            expected_x, expected_y, body->position.x, body->position.y);
    UniverseDestroy(universe);
    return 1;
  }

  // Velocity should be the same (within tolerance)
  if (fabs(mech->velocity.x - initial_vel.x) > EPSILON ||
      fabs(mech->velocity.y - initial_vel.y) > EPSILON) {
    fprintf(stderr,
            "Velocity mismatch: expected (%.2f, %.2f), got (%.2f, %.2f)\n",
            initial_vel.x, initial_vel.y, mech->velocity.x, mech->velocity.y);
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  DEBUG_PRINTF("Verlet integration test: PASSED\n");
  return 0;
}

int test_verlet_with_constant_force() {
  // PRECONDICIONES: Universo sin límites con partícula en reposo y fuerza
  // constante conocida aplicada.
  // PROCESO: Integrar una vez usando Verlet para verificar la aceleración
  // resultante por la fuerza aplicada.
  // POSTCONDICIONES: Posición y velocidad deben coincidir con la solución
  // analítica del MRUA dentro de EPSILON.

  // Create a universe
  Universe *universe = UniverseCreate(100);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return 1;
  }
  universe->boundary.enabled = false;

  KVector2 initial_pos = {0.0, 0.0};
  KVector2 initial_vel = {0.0, 0.0};
  double mass = 1.0;

  EntityID particle = UniverseCreateEntity(universe);
  if (particle != INVALID_ENTITY) {
    UniverseAddParticleComponent(universe, particle, OBJECT_RADIUS, 1.0);
    UniverseAddKineticBodyComponent(universe, particle, initial_pos, mass);
    UniverseAddMechanicsComponent(universe, particle, initial_vel);
  } else {
    fprintf(stderr, "Failed to create particle\n");
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 force = {10.0, 0.0};
  if (!PhysicsApplyConstantForce(universe, particle, force)) {
    fprintf(stderr, "Failed to register constant force\n");
    UniverseDestroy(universe);
    return 1;
  }

  // Get state
  KineticBodyComponent *body =
      UniverseGetKineticBodyComponent(universe, particle);

  MechanicsComponent *mech = UniverseGetMechanicsComponent(universe, particle);

  UniverseUpdate(universe, DELTA_TIME);

  double exp_pos_x = force.x * 0.5 * DELTA_TIME * DELTA_TIME / mass;
  double exp_pos_y = force.y * 0.5 * DELTA_TIME * DELTA_TIME / mass;
  double exp_vel_x = force.x * DELTA_TIME;
  double exp_vel_y = force.y * DELTA_TIME;

  DEBUG_PRINTF("[POSITION] After force application:\n");
  DEBUG_PRINTF("  Position: (%.6f, %.6f)\n", body->position.x,
               body->position.y);
  DEBUG_PRINTF("  Expected: (%.6f, %.6f)\n", exp_pos_x, exp_pos_y);

  DEBUG_PRINTF("[VELOCITY] After force application:\n");
  DEBUG_PRINTF("  Velocity: (%.6f, %.6f)\n", mech->velocity.x,
               mech->velocity.y);
  DEBUG_PRINTF("  Expected: (%.6f, %.6f)\n", exp_vel_x, exp_vel_y);

  if (fabs(body->position.x - exp_pos_x) > EPSILON) {
    fprintf(stderr, "Position mismatch with constant force\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(body->position.y - exp_pos_y) > EPSILON) {
    fprintf(stderr, "Position mismatch with constant force\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(mech->velocity.x - exp_vel_x) > EPSILON) {
    fprintf(stderr, "Velocity mismatch with constant force\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(mech->velocity.y - exp_vel_y) > EPSILON) {
    fprintf(stderr, "Velocity mismatch with constant force\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  DEBUG_PRINTF("Verlet with constant force test: PASSED\n");
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_verlet_integration", test_verlet_integration},
      {"test_verlet_with_constant_force", test_verlet_with_constant_force},
  };

  return RUN_TEST_SUITE("VERLET", tests);
}
