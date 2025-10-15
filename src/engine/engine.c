/**
 * engine.c
 *
 * Implementation of the Kurage Physics Engine core functionality.
 * Contains entity-component-system (ECS) logic and physics systems.
 *
 */

#include "engine.h"
#include <stdlib.h>

/* Universe Management */
// const static KVector2 ZERO_VECTOR = {0.0, 0.0};
const static KVector2 GRAVITY_VECTOR = {GRAVITY_X, GRAVITY_Y};

Universe *UniverseCreate(uint32_t maxEntities) {
  Universe *universe = (Universe *)malloc(sizeof(Universe));
  if (!universe)
    return NULL;

  // Initialize universe structure
  universe->entityCount = 0;
  universe->maxEntities = maxEntities;

  // Allocate component arrays
  universe->entityMasks =
      (ComponentMask *)calloc(maxEntities, sizeof(ComponentMask));
  universe->activeEntities = (bool *)calloc(maxEntities, sizeof(bool));
  universe->kineticBodies =
      (KineticBodyComponent *)calloc(maxEntities, sizeof(KineticBodyComponent));
  universe->mechanics =
      (MechanicsComponent *)calloc(maxEntities, sizeof(MechanicsComponent));

  // Check if allocations succeeded
  if (!universe->entityMasks || !universe->activeEntities ||
      !universe->kineticBodies || !universe->mechanics) {
    UniverseDestroy(universe);
    return NULL;
  }

  // Initialize boundary with default values
  universe->boundary.left = 0;
  universe->boundary.right = 800;
  universe->boundary.top = 0;
  universe->boundary.bottom = 600;
  universe->boundary.enabled = true;

  return universe;
}

void UniverseDestroy(Universe *universe) {
  if (!universe)
    return;

  // Free component arrays
  free(universe->entityMasks);
  free(universe->activeEntities);
  free(universe->kineticBodies);
  free(universe->mechanics);

  // Free universe structure
  free(universe);
}

void UniverseUpdate(Universe *universe, double deltaTime) {
  if (!universe)
    return;

  PhysicsForcesUpdate(universe);
  PhysicsMechanicsUpdate(universe, deltaTime);
  PhysicsPositionUpdate(universe, deltaTime);
  PhysicsClearForces(universe);

  if (universe->boundary.enabled) {
    PhysicsResolveBoundaryCollisions(universe);
  }
}

/* Entity Management */

EntityID UniverseCreateEntity(Universe *universe) {
  if (!universe)
    return INVALID_ENTITY;

  // Check if we've reached the entity limit
  if (universe->entityCount >= universe->maxEntities) {
    return INVALID_ENTITY;
  }

  // Find an available entity slot
  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i]) {
      // Initialize entity
      universe->activeEntities[i] = true;
      universe->entityMasks[i] = COMPONENT_NONE;
      universe->entityCount++;

      return i;
    }
  }

  return INVALID_ENTITY;
}

bool UniverseDestroyEntity(Universe *universe, EntityID entity) {
  if (!universe || entity >= universe->maxEntities)
    return false;

  if (universe->activeEntities[entity]) {
    // Mark entity as inactive and remove all components
    universe->activeEntities[entity] = false;
    universe->entityMasks[entity] = COMPONENT_NONE;
    universe->entityCount--;
    return true;
  }

  return false;
}

/* Component Management */

bool UniverseAddKineticBodyComponent(Universe *universe, EntityID entity,
                                     KVector2 position, double mass) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  // Add particle component
  universe->kineticBodies[entity].position = position;
  universe->kineticBodies[entity].previous = position;

  // Calculate inverse mass (handling infinite mass for static objects)
  if (mass <= 0 || isinf(mass)) {
    universe->kineticBodies[entity].inverseMass = 0.0;
  } else {
    universe->kineticBodies[entity].inverseMass = 1.0 / mass;
  }

  // Update entity's component mask
  universe->entityMasks[entity] |= COMPONENT_PARTICLE;

  return true;
}

bool UniverseAddMechanicsComponent(Universe *universe, EntityID entity,
                                   KVector2 velocity, KVector2 acceleration) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  // Add mechanics component
  universe->mechanics[entity].velocity = velocity;
  universe->mechanics[entity].acceleration = acceleration;
  universe->mechanics[entity].forceAccum.x = 0.0;
  universe->mechanics[entity].forceAccum.y = 0.0;

  // Update entity's component mask
  universe->entityMasks[entity] |= COMPONENT_MECHANICS;

  return true;
}

/* Component Access */

KineticBodyComponent *UniverseGetKineticBodyComponent(Universe *universe,
                                                EntityID entity) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return NULL;

  if (universe->entityMasks[entity] & COMPONENT_PARTICLE) {
    return &universe->kineticBodies[entity];
  }

  return NULL;
}

MechanicsComponent *UniverseGetMechanicsComponent(Universe *universe,
                                                  EntityID entity) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return NULL;

  if (universe->entityMasks[entity] & COMPONENT_MECHANICS) {
    return &universe->mechanics[entity];
  }

  return NULL;
}

/* Physics Systems */

bool PhysicsApplyForce(Universe *universe, EntityID entity, KVector2 force) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  // Check if entity has required components
  if ((universe->entityMasks[entity] &
       (COMPONENT_PARTICLE | COMPONENT_MECHANICS)) !=
      (COMPONENT_PARTICLE | COMPONENT_MECHANICS)) {
    return false;
  }

  // Accumulate force
  universe->mechanics[entity].forceAccum.x += force.x;
  universe->mechanics[entity].forceAccum.y += force.y;

  return true;
}

void PhysicsForcesUpdate(Universe *universe) {
  if (!universe)
    return;

  // Apply global gravity to all entities that can handle forces
  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (universe->activeEntities[i] &&
        (universe->entityMasks[i] &
         (COMPONENT_PARTICLE | COMPONENT_MECHANICS)) ==
            (COMPONENT_PARTICLE | COMPONENT_MECHANICS)) {

      // Apply gravity
      PhysicsApplyForce(
          universe, i,
          (KVector2){GRAVITY_VECTOR.x / universe->kineticBodies[i].inverseMass,
                     GRAVITY_VECTOR.y /
                         universe->kineticBodies[i].inverseMass});
    }
  }
}

/**
 * Update mechanics (calculate accelerations from forces, update velocities)
 */
void PhysicsMechanicsUpdate(Universe *universe, double deltaTime) {
  if (!universe)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (universe->activeEntities[i] &&
        (universe->entityMasks[i] &
         (COMPONENT_PARTICLE | COMPONENT_MECHANICS)) ==
            (COMPONENT_PARTICLE | COMPONENT_MECHANICS)) {

      // Skip entities with infinite mass
      if (universe->kineticBodies[i].inverseMass <= 0)
        continue;

      // Calculate total acceleration from forces: a = F * inverseMass
      KVector2 acceleration;
      acceleration.x = mechanics->forceAccum.x * particle->inverseMass;
      acceleration.y = mechanics->forceAccum.y * particle->inverseMass;

      // Add base acceleration
      acceleration.x += mechanics->acceleration.x;
      acceleration.y += mechanics->acceleration.y;

      // Store current position before updating (for Verlet integration)
      KVector2 currentPos = universe->kineticBodies[i].position;
      KVector2 previousPos = universe->kineticBodies[i].previous;

      // Update position using Verlet integration: new_pos = 2*current - previous + a*dt*dt
      double dt2 = deltaTime * deltaTime;
      universe->kineticBodies[i].position.x = 
          2.0 * currentPos.x - previousPos.x + acceleration.x * dt2;
      universe->kineticBodies[i].position.y = 
          2.0 * currentPos.y - previousPos.y + acceleration.y * dt2;

      // Update previous position for next iteration
      universe->kineticBodies[i].previous = currentPos;

      // Derive velocity from position change: v = (new_pos - current_pos) / dt
      universe->mechanics[i].velocity.x = (universe->kineticBodies[i].position.x - currentPos.x) / deltaTime;
      universe->mechanics[i].velocity.y = (universe->kineticBodies[i].position.y - currentPos.y) / deltaTime;

      // Reset force accumulator
      universe->mechanics[i].forceAccum.x = 0.0;
      universe->mechanics[i].forceAccum.y = 0.0;
    }
  }
}

void UniverseSetBoundaries(Universe *universe, int windowWidth,
                           int windowHeight, float padding, bool enabled) {
  if (!universe)
    return;

  universe->boundary.left = padding;
  universe->boundary.top = padding;
  universe->boundary.right = windowWidth - padding;
  universe->boundary.bottom = windowHeight - padding;
  universe->boundary.enabled = enabled;
}

void PhysicsResolveBoundaryCollisions(Universe *universe) {
  if (!universe || !universe->boundary.enabled)
    return;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    // Only process active entities with both particle and mechanics components
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] &
         (COMPONENT_PARTICLE | COMPONENT_MECHANICS)) !=
            (COMPONENT_PARTICLE | COMPONENT_MECHANICS))
      continue;

    KineticBodyComponent *particle = &universe->kineticBodies[i];
    MechanicsComponent *mechanics = &universe->mechanics[i];

    // Handle left and right boundaries
    if (particle->position.x < universe->boundary.left) {
      particle->position.x = universe->boundary.left;
      mechanics->velocity.x = -mechanics->velocity.x * RESTITUTION;
    } else if (particle->position.x > universe->boundary.right) {
      particle->position.x = universe->boundary.right;
      mechanics->velocity.x = -mechanics->velocity.x * RESTITUTION;
    }

    // Handle top and bottom boundaries
    if (particle->position.y < universe->boundary.top) {
      particle->position.y = universe->boundary.top;
      mechanics->velocity.y = -mechanics->velocity.y * RESTITUTION;
    } else if (particle->position.y > universe->boundary.bottom) {
      particle->position.y = universe->boundary.bottom;
      mechanics->velocity.y = -mechanics->velocity.y * RESTITUTION;
    }
  }
}

EntityID ParticleCreate(Universe *universe, KVector2 position,
                        KVector2 velocity, double mass) {
  EntityID entity = UniverseCreateEntity(universe);
  if (entity == INVALID_ENTITY)
    return INVALID_ENTITY;

  if (!UniverseAddKineticBodyComponent(universe, entity, position, mass)) {
    UniverseDestroyEntity(universe, entity);
    return INVALID_ENTITY;
  }

  if (!UniverseAddMechanicsComponent(universe, entity, velocity,
                                     (KVector2){0, 0})) {
    UniverseDestroyEntity(universe, entity);
    return INVALID_ENTITY;
  }

  // For Verlet integration, set previous position based on initial velocity
  // previous = position - velocity * dt
  universe->kineticBodies[entity].previous.x = position.x - velocity.x * DELTA_TIME;
  universe->kineticBodies[entity].previous.y = position.y - velocity.y * DELTA_TIME;

  return entity;
}
