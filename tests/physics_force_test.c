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

int test_apply_force_accumulates(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  EntityID entity = CreateDynamicParticle(universe, (KVector2){0.0, 0.0},
                                          (KVector2){0.0, 0.0}, 2.0);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 f1 = {4.0, -2.0};
  KVector2 f2 = {-1.0, 3.0};

  if (!PhysicsApplyForce(universe, entity, f1) ||
      !PhysicsApplyForce(universe, entity, f2)) {
    fprintf(stderr, "PhysicsApplyForce failed for valid entity\n");
    UniverseDestroy(universe);
    return 1;
  }

  MechanicsComponent *mechanics = UniverseGetMechanicsComponent(universe, entity);
  if (!mechanics) {
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 expected = {f1.x + f2.x, f1.y + f2.y};
  if (fabs(mechanics->forceAccum.x - expected.x) > EPSILON ||
      fabs(mechanics->forceAccum.y - expected.y) > EPSILON) {
    fprintf(stderr, "Force accumulator mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int test_apply_force_requires_components(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  KVector2 force = {1.0, 0.0};

  EntityID noComponents = UniverseCreateEntity(universe);
  if (noComponents == INVALID_ENTITY ||
      PhysicsApplyForce(universe, noComponents, force)) {
    fprintf(stderr, "PhysicsApplyForce should fail without components\n");
    UniverseDestroy(universe);
    return 1;
  }

  EntityID onlyMechanics = UniverseCreateEntity(universe);
  if (onlyMechanics == INVALID_ENTITY ||
      !UniverseAddMechanicsComponent(universe, onlyMechanics,
                                     (KVector2){0.0, 0.0}) ||
      PhysicsApplyForce(universe, onlyMechanics, force)) {
    fprintf(stderr, "PhysicsApplyForce should require kinetic component\n");
    UniverseDestroy(universe);
    return 1;
  }

  EntityID onlyKinetic = UniverseCreateEntity(universe);
  if (onlyKinetic == INVALID_ENTITY ||
      !UniverseAddKineticBodyComponent(universe, onlyKinetic,
                                       (KVector2){0.0, 0.0}, 1.0) ||
      PhysicsApplyForce(universe, onlyKinetic, force)) {
    fprintf(stderr, "PhysicsApplyForce should require mechanics component\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int test_constant_force_accumulation(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  EntityID entity = CreateDynamicParticle(universe, (KVector2){0.0, 0.0},
                                          (KVector2){0.0, 0.0}, 1.0);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 constant = {2.0, -1.0};
  KVector2 extra = {-1.5, 0.5};

  if (!PhysicsApplyConstantForce(universe, entity, constant) ||
      !PhysicsApplyConstantForce(universe, entity, extra)) {
    fprintf(stderr, "PhysicsApplyConstantForce failed\n");
    UniverseDestroy(universe);
    return 1;
  }

  MechanicsComponent *mechanics = UniverseGetMechanicsComponent(universe, entity);
  if (!mechanics) {
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 expectedConstant = {constant.x + extra.x, constant.y + extra.y};
  if (fabs(mechanics->constantForces.x - expectedConstant.x) > EPSILON ||
      fabs(mechanics->constantForces.y - expectedConstant.y) > EPSILON) {
    fprintf(stderr, "Constant force accumulator mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!PhysicsRemoveConstantForce(universe, entity, constant)) {
    fprintf(stderr, "PhysicsRemoveConstantForce failed\n");
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 expectedAfterRemoval = {expectedConstant.x - constant.x,
                                   expectedConstant.y - constant.y};
  if (fabs(mechanics->constantForces.x - expectedAfterRemoval.x) > EPSILON ||
      fabs(mechanics->constantForces.y - expectedAfterRemoval.y) > EPSILON) {
    fprintf(stderr, "Constant force removal mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int test_forces_update_and_clear(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  EntityID entity = CreateDynamicParticle(universe, (KVector2){0.0, 0.0},
                                          (KVector2){0.0, 0.0}, 1.0);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  MechanicsComponent *mechanics = UniverseGetMechanicsComponent(universe, entity);
  if (!mechanics) {
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 constant = {3.0, -1.0};
  KVector2 impulseForce = {2.0, 2.0};

  if (!PhysicsApplyConstantForce(universe, entity, constant) ||
      !PhysicsApplyForce(universe, entity, impulseForce)) {
    fprintf(stderr, "Failed to setup forces\n");
    UniverseDestroy(universe);
    return 1;
  }

  PhysicsForcesUpdate(universe);

  KVector2 expectedAfterUpdate = {impulseForce.x + constant.x,
                                  impulseForce.y + constant.y};
  if (fabs(mechanics->forceAccum.x - expectedAfterUpdate.x) > EPSILON ||
      fabs(mechanics->forceAccum.y - expectedAfterUpdate.y) > EPSILON) {
    fprintf(stderr, "Force update mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  PhysicsClearForces(universe);
  if (fabs(mechanics->forceAccum.x) > EPSILON ||
      fabs(mechanics->forceAccum.y) > EPSILON) {
    fprintf(stderr, "Force accumulator should be cleared\n");
    UniverseDestroy(universe);
    return 1;
  }

  PhysicsForcesUpdate(universe);
  if (fabs(mechanics->forceAccum.x - constant.x) > EPSILON ||
      fabs(mechanics->forceAccum.y - constant.y) > EPSILON) {
    fprintf(stderr, "Constant force should be re-applied\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int test_apply_impulse_updates_velocity(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  EntityID entity = CreateDynamicParticle(universe, (KVector2){0.0, 0.0},
                                          (KVector2){1.0, -1.0}, 2.0);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  MechanicsComponent *mechanics = UniverseGetMechanicsComponent(universe, entity);
  KineticBodyComponent *body = UniverseGetKineticBodyComponent(universe, entity);
  if (!mechanics || !body) {
    UniverseDestroy(universe);
    return 1;
  }

  KVector2 impulse = {2.0, 4.0};
  if (!PhysicsApplyImpulse(universe, entity, impulse)) {
    fprintf(stderr, "PhysicsApplyImpulse failed\n");
    UniverseDestroy(universe);
    return 1;
  }

  double invMass = body->inverseMass;
  KVector2 expectedVelocity = {1.0 + impulse.x * invMass,
                               -1.0 + impulse.y * invMass};

  if (fabs(mechanics->velocity.x - expectedVelocity.x) > EPSILON ||
      fabs(mechanics->velocity.y - expectedVelocity.y) > EPSILON) {
    fprintf(stderr, "Impulse velocity mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!mechanics->needsVerletSync) {
    fprintf(stderr, "Impulse should mark Verlet sync\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_apply_force_accumulates", test_apply_force_accumulates},
      {"test_apply_force_requires_components", test_apply_force_requires_components},
      {"test_constant_force_accumulation", test_constant_force_accumulation},
      {"test_forces_update_and_clear", test_forces_update_and_clear},
      {"test_apply_impulse_updates_velocity", test_apply_impulse_updates_velocity},
  };

  return RUN_TEST_SUITE("PHYSICS_FORCES", tests);
}
