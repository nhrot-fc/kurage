#include "systems.h"

static const KVector2 GRAVITY_VECTOR = {GRAVITY_X, GRAVITY_Y};

bool PhysicsApplyForce(Universe *universe, EntityID entity, KVector2 force) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  ComponentMask required = COMPONENT_PARTICLE | COMPONENT_MECHANICS;
  if ((universe->entityMasks[entity] & required) != required)
    return false;

  universe->mechanics[entity].forceAccum.x += force.x;
  universe->mechanics[entity].forceAccum.y += force.y;
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

    double inverseMass = universe->kineticBodies[i].inverseMass;
    if (inverseMass <= 0.0)
      continue;

    // double mass = 1.0 / inverseMass;
    // universe->mechanics[i].forceAccum.x += mass * GRAVITY_VECTOR.x;
    // universe->mechanics[i].forceAccum.y += mass * GRAVITY_VECTOR.y;
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

    if (universe->kineticBodies[i].inverseMass <= 0)
      continue;

    KineticBodyComponent *particle = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];

    KVector2 acceleration;
    acceleration.x = mechanics->forceAccum.x * particle->inverseMass;
    acceleration.y = mechanics->forceAccum.y * particle->inverseMass;
    acceleration.x += mechanics->acceleration.x;
    acceleration.y += mechanics->acceleration.y;

    mechanics->velocity.x += acceleration.x * deltaTime;
    mechanics->velocity.y += acceleration.y * deltaTime;
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
    particle->position.x += mechanics->velocity.x * deltaTime;
    particle->position.y += mechanics->velocity.y * deltaTime;
  }
}

void PhysicsClearForces(Universe *universe) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i] ||
        !(universe->entityMasks[i] & COMPONENT_MECHANICS))
      continue;

    universe->mechanics[i].forceAccum.x = 0.0;
    universe->mechanics[i].forceAccum.y = 0.0;
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

    if (particle->position.x < universe->boundary.left) {
      particle->position.x = universe->boundary.left;
      mechanics->velocity.x = -mechanics->velocity.x * RESTITUTION;
    } else if (particle->position.x > universe->boundary.right) {
      particle->position.x = universe->boundary.right;
      mechanics->velocity.x = -mechanics->velocity.x * RESTITUTION;
    }

    if (particle->position.y < universe->boundary.top) {
      particle->position.y = universe->boundary.top;
      mechanics->velocity.y = -mechanics->velocity.y * RESTITUTION;
    } else if (particle->position.y > universe->boundary.bottom) {
      particle->position.y = universe->boundary.bottom;
      mechanics->velocity.y = -mechanics->velocity.y * RESTITUTION;
    }
  }
}