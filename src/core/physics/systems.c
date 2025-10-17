#include "systems.h"

#include <math.h>

static const size_t COLLISION_MAX_NEIGHBORS = 1000;
static const double COLLISION_BAUMGARTE = 0.01;
static const double COLLISION_PENETRATION_SLOP = 0.01;

static const KVector2 GRAVITY_VECTOR = {GRAVITY_X, GRAVITY_Y};

bool PhysicsApplyForce(Universe *universe, EntityID entity, KVector2 force) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
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
    ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    KineticBodyComponent *body = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];

    double inverseMass = body->inverseMass;
    if (inverseMass <= 0.0)
      continue;

    double mass = 1.0 / inverseMass;
    KVector2 gravityForce = KVector2ScalarProduct(GRAVITY_VECTOR, mass);
    mechanics->forceAccum =
        KVector2Addition(mechanics->forceAccum, gravityForce);
  }
}

void PhysicsMechanicsUpdate(Universe *universe, double deltaTime) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
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
    ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
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

void PhysicsResolveParticleCollisions(Universe *universe, double deltaTime) {
  if (!universe || deltaTime <= 0.0)
    return;

  const ComponentMask required =
      COMPONENT_KINETIC | COMPONENT_MECHANICS | COMPONENT_PARTICLE;

  EntityID neighbors[COLLISION_MAX_NEIGHBORS];

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    KineticBodyComponent *bodyA = &universe->kineticBodies[i];
    MechanicsComponent *mechA = &universe->mechanics[i];
    ParticleComponent *shapeA = &universe->particles[i];

    size_t neighborCount =
        UniverseQueryNeighbors(universe, bodyA->position, shapeA->radius,
                               neighbors, COLLISION_MAX_NEIGHBORS);

    for (size_t n = 0; n < neighborCount; n++) {
      EntityID j = neighbors[n];
      if (j == i || j >= universe->maxEntities)
        continue;

      if (!universe->activeEntities[j] ||
          (universe->entityMasks[j] & required) != required)
        continue;

      if (j < i)
        continue;

      KineticBodyComponent *bodyB = &universe->kineticBodies[j];
      MechanicsComponent *mechB = &universe->mechanics[j];
      ParticleComponent *shapeB = &universe->particles[j];

      double invMassA = bodyA->inverseMass;
      double invMassB = bodyB->inverseMass;
      double invMassSum = invMassA + invMassB;
      if (invMassSum <= 0.0)
        continue;

      double dx = bodyB->position.x - bodyA->position.x;
      double dy = bodyB->position.y - bodyA->position.y;
      double distanceSq = dx * dx + dy * dy;

      double combinedRadius = shapeA->radius + shapeB->radius;
      if (combinedRadius <= 0.0)
        continue;
      double combinedRadiusSq = combinedRadius * combinedRadius;
      if (distanceSq >= combinedRadiusSq)
        continue;

      double distance = sqrt(distanceSq);
      KVector2 normal = {0.0, 0.0};
      if (distance > 1e-6) {
        normal.x = dx / distance;
        normal.y = dy / distance;
      } else {
        normal.x = 1.0;
        normal.y = 0.0;
        distance = 0.0;
      }

      double penetration = combinedRadius - distance;
      if (penetration <= 0.0)
        continue;

      double penetrationDepth = penetration - COLLISION_PENETRATION_SLOP;
      if (penetrationDepth < 0.0)
        penetrationDepth = 0.0;

      double positionFactorA = invMassA / invMassSum;
      double positionFactorB = invMassB / invMassSum;
      bodyA->position.x -= normal.x * penetrationDepth * positionFactorA;
      bodyA->position.y -= normal.y * penetrationDepth * positionFactorA;
      bodyB->position.x += normal.x * penetrationDepth * positionFactorB;
      bodyB->position.y += normal.y * penetrationDepth * positionFactorB;

      KVector2 relativeVelocity = {mechB->velocity.x - mechA->velocity.x,
                                   mechB->velocity.y - mechA->velocity.y};
      double velocityAlongNormal =
          relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

      double biasVelocity =
          (COLLISION_BAUMGARTE / deltaTime) * penetrationDepth;

      double adjustedRelative = velocityAlongNormal - biasVelocity;

      if (adjustedRelative > 0.0)
        continue;

      double impulseMag = -(1.0 + RESTITUTION) * adjustedRelative;
      impulseMag /= invMassSum;

      if (impulseMag < 0.0)
        impulseMag = 0.0;

      double impulseX = impulseMag * normal.x;
      double impulseY = impulseMag * normal.y;

      mechA->velocity.x -= impulseX * invMassA;
      mechA->velocity.y -= impulseY * invMassA;
      mechB->velocity.x += impulseX * invMassB;
      mechB->velocity.y += impulseY * invMassB;
    }
  }
}

void PhysicsResolveBoundaryCollisions(Universe *universe) {
  if (!universe || !universe->boundary.enabled)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required =
        COMPONENT_KINETIC | COMPONENT_MECHANICS | COMPONENT_PARTICLE;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    KineticBodyComponent *particle = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];
    ParticleComponent *shape = &universe->particles[i];

    double minX = universe->boundary.left + shape->radius;
    double maxX = universe->boundary.right - shape->radius;
    if (minX > maxX) {
      double mid = (universe->boundary.left + universe->boundary.right) * 0.5;
      minX = mid;
      maxX = mid;
    }

    if (particle->position.x < minX) {
      particle->position.x = minX;
      mechanics->velocity.x = -mechanics->velocity.x * RESTITUTION;
    } else if (particle->position.x > maxX) {
      particle->position.x = maxX;
      mechanics->velocity.x = -mechanics->velocity.x * RESTITUTION;
    }

    double minY = universe->boundary.top + shape->radius;
    double maxY = universe->boundary.bottom - shape->radius;
    if (minY > maxY) {
      double midY = (universe->boundary.top + universe->boundary.bottom) * 0.5;
      minY = midY;
      maxY = midY;
    }

    if (particle->position.y < minY) {
      particle->position.y = minY;
      mechanics->velocity.y = -mechanics->velocity.y * RESTITUTION;
    } else if (particle->position.y > maxY) {
      particle->position.y = maxY;
      mechanics->velocity.y = -mechanics->velocity.y * RESTITUTION;
    }
  }
}