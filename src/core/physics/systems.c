#include "systems.h"

#include <math.h>

static const size_t COLLISION_MAX_NEIGHBORS = 1000;
static const double COLLISION_PENETRATION_SLOP = 0.01;

/**
 * Newton's second law: F = m * a. The force is stored so that
 * a = F * inverseMass can be applied during the integration step.
 */
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

/**
 * Total force balance: F_total = ΣF_constant + ΣF_instant. The constant
 * contribution is accumulated so it is re-applied every substep.
 */
bool PhysicsApplyConstantForce(Universe *universe, EntityID entity,
                               KVector2 force) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
  if ((universe->entityMasks[entity] & required) != required)
    return false;

  MechanicsComponent *mechanics = &universe->mechanics[entity];
  mechanics->constantForces =
      KVector2Addition(mechanics->constantForces, force);
  return true;
}

/**
 * Removes a constant force contribution so that F_total reflects the
 * updated sum ΣF_constant.
 */
bool PhysicsRemoveConstantForce(Universe *universe, EntityID entity,
                                KVector2 force) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
  if ((universe->entityMasks[entity] & required) != required)
    return false;

  MechanicsComponent *mechanics = &universe->mechanics[entity];
  mechanics->constantForces =
      KVector2Subtraction(mechanics->constantForces, force);
  return true;
}

/**
 * Impulse-momentum relation: J = Δp = m * Δv. Applying impulse J updates
 * velocity by Δv = J * inverseMass and flags the integrator so the next
 * Verlet step re-synchronizes the history with the new state.
 */
bool PhysicsApplyImpulse(Universe *universe, EntityID entity,
                         KVector2 impulse) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
  if ((universe->entityMasks[entity] & required) != required)
    return false;

  KineticBodyComponent *body = &universe->kineticBodies[entity];
  if (body->inverseMass <= 0.0)
    return false;

  MechanicsComponent *mechanics = &universe->mechanics[entity];
  KVector2 deltaVelocity = KVector2ScalarProduct(impulse, body->inverseMass);
  mechanics->velocity = KVector2Addition(mechanics->velocity, deltaVelocity);
  mechanics->needsVerletSync = true;
  return true;
}

/**
 * Ensures each substep applies F_total = F_accum + F_constant by adding the
 * persistent forces before computing acceleration.
 */
void PhysicsForcesUpdate(Universe *universe) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    MechanicsComponent *mechanics = &universe->mechanics[i];

    mechanics->forceAccum =
        KVector2Addition(mechanics->forceAccum, mechanics->constantForces);
  }
}

/**
 * Newton's second law: a = F / m. Stores the acceleration used by the
 * position Verlet integrator.
 */
void PhysicsMechanicsUpdate(Universe *universe, double deltaTime) {
  if (!universe)
    return;

  (void)deltaTime;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    KineticBodyComponent *particle = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];

    if (particle->inverseMass <= 0.0) {
      mechanics->acceleration = (KVector2){0.0, 0.0};
      continue;
    }

    mechanics->acceleration =
        KVector2ScalarProduct(mechanics->forceAccum, particle->inverseMass);
  }
}

/**
 * Position Verlet step: x_{n+1} = 2 x_n - x_{n-1} + a_n * (Δt)^2. Velocity is
 * advanced with v_{n+1} = v_n + a_n * Δt and used for sync when impulses occur.
 */
void PhysicsPositionUpdate(Universe *universe, double deltaTime) {
  if (!universe)
    return;

  if (deltaTime <= 0.0)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_KINETIC | COMPONENT_MECHANICS;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    KineticBodyComponent *particle = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];

    if (particle->inverseMass <= 0.0) {
      mechanics->acceleration = (KVector2){0.0, 0.0};
      mechanics->velocity = (KVector2){0.0, 0.0};
      particle->previous = particle->position;
      mechanics->needsVerletSync = false;
      continue;
    }

    if (mechanics->needsVerletSync) {
      double dtSq = deltaTime * deltaTime;
      double halfDtSq = 0.5 * dtSq;
      KVector2 velocityTerm =
          KVector2ScalarProduct(mechanics->velocity, deltaTime);
      KVector2 halfAccelerationTerm =
          KVector2ScalarProduct(mechanics->acceleration, halfDtSq);

      particle->previous =
          KVector2Subtraction(particle->position, velocityTerm);
      particle->previous =
          KVector2Addition(particle->previous, halfAccelerationTerm);
      mechanics->needsVerletSync = false;
    }

    KVector2 prevPosition = particle->previous;
    KVector2 currentPosition = particle->position;

    double dtSq = deltaTime * deltaTime;
    KVector2 inertiaTerm = KVector2Subtraction(currentPosition, prevPosition);
    KVector2 accelerationTerm =
        KVector2ScalarProduct(mechanics->acceleration, dtSq);

    KVector2 newPosition = KVector2Addition(currentPosition, inertiaTerm);
    newPosition = KVector2Addition(newPosition, accelerationTerm);

    particle->previous = currentPosition;
    particle->position = newPosition;

    KVector2 velocityDelta =
        KVector2ScalarProduct(mechanics->acceleration, deltaTime);
    mechanics->velocity = KVector2Addition(mechanics->velocity, velocityDelta);
  }
}

/**
 * Prepares the next simulation step by resetting the accumulator so that
 * F_accum(next) = 0 before new forces are registered.
 */
void PhysicsClearForces(Universe *universe) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i] ||
        !(universe->entityMasks[i] & COMPONENT_MECHANICS))
      continue;

    MechanicsComponent *mechanics = &universe->mechanics[i];
    mechanics->forceAccum = (KVector2){0, 0};
  }
}

/**
 * Resolves particle contacts using positional correction
 * Δx = penetrationDepth * inverseMass / (inverseMassA + inverseMassB)
 * and impulse magnitude
 * j = (-(1 + e) * (v_rel·n) + bias) / (inverseMassA + inverseMassB).
 */
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

      double biasVelocity = 0.0;

      if (velocityAlongNormal > 0.0 && penetrationDepth <= 0.0)
        continue;

      double impulseMag = -(2.0) * velocityAlongNormal;
      impulseMag -= biasVelocity;
      impulseMag /= invMassSum;

      if (impulseMag <= 0.0)
        continue;

      double impulseX = impulseMag * normal.x;
      double impulseY = impulseMag * normal.y;

      mechA->velocity.x -= impulseX * invMassA;
      mechA->velocity.y -= impulseY * invMassA;
      mechB->velocity.x += impulseX * invMassB;
      mechB->velocity.y += impulseY * invMassB;

      mechA->needsVerletSync = true;
      mechB->needsVerletSync = true;
    }
  }
}

/**
 * Clamps particles inside the axis-aligned bounds and reflects velocity with
 * v_out = -e * v_in along the colliding axis, where e is the restitution.
 */
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
      mechanics->needsVerletSync = true;
    } else if (particle->position.x > maxX) {
      particle->position.x = maxX;
      mechanics->velocity.x = -mechanics->velocity.x * RESTITUTION;
      mechanics->needsVerletSync = true;
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
      mechanics->needsVerletSync = true;
    } else if (particle->position.y > maxY) {
      particle->position.y = maxY;
      mechanics->velocity.y = -mechanics->velocity.y * RESTITUTION;
      mechanics->needsVerletSync = true;
    }
  }
}