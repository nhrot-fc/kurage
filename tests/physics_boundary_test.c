#include "../src/core/physics/systems.h"
#include "test_framework.h"

#include <math.h>
#include <stdio.h>

#define EPSILON 1e-6

static Universe *CreateTestUniverse(void) {
  Universe *universe = UniverseCreate(16);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return NULL;
  }
  universe->boundary.enabled = true;
  universe->boundary.left = -10.0;
  universe->boundary.right = 10.0;
  universe->boundary.top = -10.0;
  universe->boundary.bottom = 10.0;
  return universe;
}

static EntityID CreateDynamicParticle(Universe *universe, KVector2 position,
                                      KVector2 velocity, double mass) {
  EntityID entity = UniverseCreateEntity(universe);
  if (entity == INVALID_ENTITY)
    return INVALID_ENTITY;

  if (!UniverseAddParticleComponent(universe, entity, OBJECT_RADIUS, 1.0))
    return INVALID_ENTITY;
  if (!UniverseAddKineticBodyComponent(universe, entity, position, mass))
    return INVALID_ENTITY;
  if (!UniverseAddMechanicsComponent(universe, entity, velocity))
    return INVALID_ENTITY;

  return entity;
}

int test_boundary_reflects_horizontal_velocity(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  double leftLimit = universe->boundary.left + OBJECT_RADIUS;
  double initialVelX = -3.0;
  EntityID entity = CreateDynamicParticle(
    universe, (KVector2){leftLimit - 1.0, 0.0},
    (KVector2){initialVelX, 0.5}, 1.0);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  PhysicsResolveBoundaryCollisions(universe);

  MechanicsComponent *mechanics =
      UniverseGetMechanicsComponent(universe, entity);
  KineticBodyComponent *body =
      UniverseGetKineticBodyComponent(universe, entity);
  if (!mechanics || !body) {
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(body->position.x - leftLimit) > EPSILON) {
    fprintf(stderr, "Boundary did not clamp position\n");
    UniverseDestroy(universe);
    return 1;
  }

  double expectedVelX = fabs(initialVelX) * RESTITUTION;
  if (fabs(mechanics->velocity.x - expectedVelX) > EPSILON) {
    fprintf(stderr, "Boundary did not reflect horizontal velocity\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!mechanics->needsVerletSync) {
    fprintf(stderr, "Boundary collision should flag Verlet sync\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int test_boundary_reflects_vertical_velocity(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  double topLimit = universe->boundary.top + OBJECT_RADIUS;
  double initialVelY = -2.5;
  EntityID entity = CreateDynamicParticle(
    universe, (KVector2){0.0, topLimit - 0.5},
    (KVector2){1.0, initialVelY}, 1.0);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  PhysicsResolveBoundaryCollisions(universe);

  MechanicsComponent *mechanics =
      UniverseGetMechanicsComponent(universe, entity);
  KineticBodyComponent *body =
      UniverseGetKineticBodyComponent(universe, entity);
  if (!mechanics || !body) {
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(body->position.y - topLimit) > EPSILON) {
    fprintf(stderr, "Boundary did not clamp vertical position\n");
    UniverseDestroy(universe);
    return 1;
  }

  double expectedVelY = fabs(initialVelY) * RESTITUTION;
  if (fabs(mechanics->velocity.y - expectedVelY) > EPSILON) {
    fprintf(stderr, "Boundary did not reflect vertical velocity\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_boundary_reflects_horizontal_velocity",
       test_boundary_reflects_horizontal_velocity},
      {"test_boundary_reflects_vertical_velocity",
       test_boundary_reflects_vertical_velocity},
  };

  return RUN_TEST_SUITE("PHYSICS_BOUNDARY", tests);
}
