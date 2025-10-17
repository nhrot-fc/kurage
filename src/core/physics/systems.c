#include "systems.h"

static const KVector2 GRAVITY_VECTOR = {GRAVITY_X, GRAVITY_Y};

bool PhysicsApplyForce(Universe *universe, EntityID entity, KVector2 force) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  ComponentMask required = COMPONENT_PARTICLE | COMPONENT_MECHANICS;
  if ((universe->entityMasks[entity] & required) != required)
    return false;

  MechanicsComponent *mechanics = &universe->mechanics[entity];
  mechanics->forceAccum = KVector2Addition(mechanics->forceAccum, force);
  return true;
}

void PhysicsForcesUpdate(Universe *universe) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_PARTICLE | COMPONENT_MECHANICS;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    // KineticBodyComponent *particle = &universe->kineticBodies[i];
    // MechanicsComponent *mechanics = &universe->mechanics[i];

    // double inverseMass = particle->inverseMass;
    // if (inverseMass <= 0.0)
    //   continue;

    // double mass = 1.0 / inverseMass;
    // KVector2 gravityForce = KVector2ScalarProduct(GRAVITY_VECTOR, mass);
    // mechanics->forceAccum =
    //     KVector2Addition(mechanics->forceAccum, gravityForce);
  }
}

void PhysicsMechanicsUpdate(Universe *universe, double deltaTime) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_PARTICLE | COMPONENT_MECHANICS;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    if (universe->kineticBodies[i].inverseMass <= 0.0)
      continue;

    KineticBodyComponent *particle = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];

    KVector2 forceAcceleration =
        KVector2ScalarProduct(mechanics->forceAccum, particle->inverseMass);
    KVector2 acceleration =
        KVector2Addition(forceAcceleration, mechanics->acceleration);

    KVector2 velocityDelta = KVector2ScalarProduct(acceleration, deltaTime);
    mechanics->velocity = KVector2Addition(mechanics->velocity, velocityDelta);
  }
}

void PhysicsPositionUpdate(Universe *universe, double deltaTime) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_PARTICLE | COMPONENT_MECHANICS;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    KineticBodyComponent *particle = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];

    particle->previous = particle->position;
    KVector2 displacement =
        KVector2ScalarProduct(mechanics->velocity, deltaTime);
    particle->position = KVector2Addition(particle->position, displacement);
  }
}

void PhysicsClearForces(Universe *universe) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i] ||
        !(universe->entityMasks[i] & COMPONENT_MECHANICS))
      continue;

    MechanicsComponent *mechanics = &universe->mechanics[i];
    mechanics->forceAccum = KVector2ScalarProduct(mechanics->forceAccum, 0.0);
  }
}

void PhysicsResolveBoundaryCollisions(Universe *universe) {
  if (!universe || !universe->boundary.enabled)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_PARTICLE | COMPONENT_MECHANICS;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    KineticBodyComponent *particle = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];

    double clampedX = KClamp(particle->position.x, universe->boundary.left,
                             universe->boundary.right);
    if (clampedX != particle->position.x) {
      particle->position.x = clampedX;
      mechanics->velocity.x = -mechanics->velocity.x * RESTITUTION;
    }

    double clampedY = KClamp(particle->position.y, universe->boundary.top,
                             universe->boundary.bottom);
    if (clampedY != particle->position.y) {
      particle->position.y = clampedY;
      mechanics->velocity.y = -mechanics->velocity.y * RESTITUTION;
    }
  }
}