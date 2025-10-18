#include "../src/core/physics/systems.h"
#include "test_framework.h"

#include <math.h>
#include <stdio.h>

#define EPSILON 1e-6

static Universe *CreateTestUniverse(void) {
  Universe *universe = UniverseCreate(32);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return NULL;
  }
  universe->boundary.enabled = false;
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

int test_mechanics_acceleration_computation(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  EntityID entity = CreateDynamicParticle(universe, (KVector2){0.0, 0.0},
                                          (KVector2){0.0, 0.0}, 2.0);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 force = {10.0, -4.0};
  if (!PhysicsApplyForce(universe, entity, force)) {
    fprintf(stderr, "PhysicsApplyForce failed\n");
    UniverseDestroy(universe);
    return 1;
  }

  double dt = 0.1;
  PhysicsMechanicsUpdate(universe, dt);

  MechanicsComponent *mechanics = UniverseGetMechanicsComponent(universe, entity);
  KineticBodyComponent *body = UniverseGetKineticBodyComponent(universe, entity);
  if (!mechanics || !body) {
    UniverseDestroy(universe);
    return 1;
  }

  double invMass = body->inverseMass;
  KVector2 expectedAccel = {force.x * invMass, force.y * invMass};

  if (fabs(mechanics->acceleration.x - expectedAccel.x) > EPSILON ||
      fabs(mechanics->acceleration.y - expectedAccel.y) > EPSILON) {
    fprintf(stderr, "Acceleration mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int test_position_update_verlet_motion(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  KVector2 initialPosition = {0.0, 0.0};
  KVector2 initialVelocity = {2.0, 1.0};
  double mass = 1.0;

  EntityID entity = CreateDynamicParticle(universe, initialPosition,
                                          initialVelocity, mass);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 constantForce = {4.0, -2.0};
  if (!PhysicsApplyConstantForce(universe, entity, constantForce)) {
    fprintf(stderr, "Failed to apply constant force\n");
    UniverseDestroy(universe);
    return 1;
  }

  double dt = 0.1;
  PhysicsForcesUpdate(universe);
  PhysicsMechanicsUpdate(universe, dt);
  PhysicsPositionUpdate(universe, dt);

  MechanicsComponent *mechanics = UniverseGetMechanicsComponent(universe, entity);
  KineticBodyComponent *body = UniverseGetKineticBodyComponent(universe, entity);
  if (!mechanics || !body) {
    UniverseDestroy(universe);
    return 1;
  }

  double invMass = body->inverseMass;
  double accelerationX = constantForce.x * invMass;
  double accelerationY = constantForce.y * invMass;

  double expectedPosX = initialPosition.x + initialVelocity.x * dt + 0.5 * accelerationX * dt * dt;
  double expectedPosY = initialPosition.y + initialVelocity.y * dt + 0.5 * accelerationY * dt * dt;
  double expectedVelX = initialVelocity.x + accelerationX * dt;
  double expectedVelY = initialVelocity.y + accelerationY * dt;

  if (fabs(body->position.x - expectedPosX) > 1e-4 ||
      fabs(body->position.y - expectedPosY) > 1e-4) {
    fprintf(stderr, "Position integration mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(mechanics->velocity.x - expectedVelX) > 1e-5 ||
      fabs(mechanics->velocity.y - expectedVelY) > 1e-5) {
    fprintf(stderr, "Velocity integration mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (mechanics->needsVerletSync) {
    fprintf(stderr, "Verlet sync flag should be cleared\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int test_static_body_ignores_forces(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  EntityID entity = CreateDynamicParticle(universe, (KVector2){5.0, 5.0},
                                          (KVector2){0.0, 0.0}, 0.0);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  if (!PhysicsApplyForce(universe, entity, (KVector2){10.0, 10.0}) ||
      !PhysicsApplyConstantForce(universe, entity, (KVector2){5.0, -3.0})) {
    fprintf(stderr, "Failed to apply forces\n");
    UniverseDestroy(universe);
    return 1;
  }

  KineticBodyComponent *body = UniverseGetKineticBodyComponent(universe, entity);
  MechanicsComponent *mechanics = UniverseGetMechanicsComponent(universe, entity);

  if (!body || !mechanics) {
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(body->inverseMass) > EPSILON) {
    fprintf(stderr, "Static body should have zero inverse mass\n");
    UniverseDestroy(universe);
    return 1;
  }

  double dt = 0.1;
  PhysicsForcesUpdate(universe);
  PhysicsMechanicsUpdate(universe, dt);
  PhysicsPositionUpdate(universe, dt);

  if (fabs(mechanics->acceleration.x) > EPSILON ||
      fabs(mechanics->acceleration.y) > EPSILON) {
    fprintf(stderr, "Static body should not accelerate\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(body->position.x - 5.0) > EPSILON ||
      fabs(body->position.y - 5.0) > EPSILON) {
    fprintf(stderr, "Static body position should remain fixed\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(mechanics->velocity.x) > EPSILON ||
      fabs(mechanics->velocity.y) > EPSILON) {
    fprintf(stderr, "Static body velocity should remain zero\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_mechanics_acceleration_computation", test_mechanics_acceleration_computation},
      {"test_position_update_verlet_motion", test_position_update_verlet_motion},
      {"test_static_body_ignores_forces", test_static_body_ignores_forces},
  };

  return RUN_TEST_SUITE("PHYSICS_MECHANICS", tests);
}
