#include "../src/core/physics/systems.h"
#include "test_framework.h"

#include <math.h>
#include <stdio.h>

#define EPSILON 1e-5

static Universe *CreateTestUniverse(void) {
  Universe *universe = UniverseCreate(64);
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

static double KineticEnergy(const KineticBodyComponent *body,
                            const MechanicsComponent *mech) {
  if (body->inverseMass <= 0.0)
    return 0.0;
  double mass = 1.0 / body->inverseMass;
  double speedSq = mech->velocity.x * mech->velocity.x +
                   mech->velocity.y * mech->velocity.y;
  return 0.5 * mass * speedSq;
}

int test_particle_collision_velocity_exchange(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  KVector2 posA = {-4.9, 0.0};
  KVector2 posB = {4.9, 0.0};
  KVector2 velA = {1.5, 0.0};
  KVector2 velB = {-1.0, 0.0};

  EntityID entityA = CreateDynamicParticle(universe, posA, velA, 1.0);
  EntityID entityB = CreateDynamicParticle(universe, posB, velB, 1.0);
  if (entityA == INVALID_ENTITY || entityB == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  MechanicsComponent *mechanicsA =
    UniverseGetMechanicsComponent(universe, entityA);
  MechanicsComponent *mechanicsB =
    UniverseGetMechanicsComponent(universe, entityB);
  KineticBodyComponent *bodyA = UniverseGetKineticBodyComponent(universe, entityA);
  KineticBodyComponent *bodyB = UniverseGetKineticBodyComponent(universe, entityB);

  if (!mechanicsA || !mechanicsB || !bodyA || !bodyB) {
    UniverseDestroy(universe);
    return 1;
  }

  double energyBefore = KineticEnergy(bodyA, mechanicsA) +
                        KineticEnergy(bodyB, mechanicsB);

  UniverseUpdateSpatialGrid(universe);

  double dt = 0.1;
  PhysicsResolveParticleCollisions(universe, dt);

  double energyAfter = KineticEnergy(bodyA, mechanicsA) +
                       KineticEnergy(bodyB, mechanicsB);

  double energyTolerance = fmax(1e-3, energyBefore * 0.05);
  double energyDiff = fabs(energyAfter - energyBefore);
  if (energyDiff > energyTolerance) {
    fprintf(stderr,
            "Kinetic energy mismatch: before=%.6f after=%.6f diff=%.6f\n",
            energyBefore, energyAfter, energyDiff);
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(mechanicsA->velocity.x - velB.x) > 5e-3 ||
      fabs(mechanicsB->velocity.x - velA.x) > 5e-3) {
    fprintf(stderr, "Velocities did not exchange as expected\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!mechanicsA->needsVerletSync || !mechanicsB->needsVerletSync) {
    fprintf(stderr, "Collision should request Verlet re-sync\n");
    UniverseDestroy(universe);
    return 1;
  }

  double dx = bodyB->position.x - bodyA->position.x;
  double dy = bodyB->position.y - bodyA->position.y;
  double distance = sqrt(dx * dx + dy * dy);
  double combinedRadius = OBJECT_RADIUS * 2.0;
  if (distance + 0.05 < combinedRadius) {
    fprintf(stderr, "Particles remain deeply penetrated\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int test_particle_collision_requires_components(void) {
  Universe *universe = CreateTestUniverse();
  if (!universe)
    return 1;

  EntityID entity = UniverseCreateEntity(universe);
  if (entity == INVALID_ENTITY) {
    UniverseDestroy(universe);
    return 1;
  }

  UniverseUpdateSpatialGrid(universe);
  PhysicsResolveParticleCollisions(universe, 0.016);

  UniverseDestroy(universe);
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_particle_collision_velocity_exchange",
       test_particle_collision_velocity_exchange},
      {"test_particle_collision_requires_components",
       test_particle_collision_requires_components},
  };

  return RUN_TEST_SUITE("PHYSICS_COLLISIONS", tests);
}
