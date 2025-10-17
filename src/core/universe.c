#include "universe.h"

#include <math.h>
#include <stdlib.h>

Universe *UniverseCreate(uint32_t maxEntities) {
  Universe *universe = (Universe *)malloc(sizeof(Universe));
  if (!universe)
    return NULL;

  universe->entityCount = 0;
  universe->maxEntities = maxEntities;

  universe->entityMasks =
      (ComponentMask *)calloc(maxEntities, sizeof(ComponentMask));
  universe->activeEntities = (bool *)calloc(maxEntities, sizeof(bool));
  universe->kineticBodies =
      (KineticBodyComponent *)calloc(maxEntities, sizeof(KineticBodyComponent));
  universe->mechanics =
      (MechanicsComponent *)calloc(maxEntities, sizeof(MechanicsComponent));

  if (!universe->entityMasks || !universe->activeEntities ||
      !universe->kineticBodies || !universe->mechanics) {
    UniverseDestroy(universe);
    return NULL;
  }

  universe->boundary.left = BOUNDARY_PADDING;
  universe->boundary.top = BOUNDARY_PADDING;
  universe->boundary.right = WINDOW_DEFAULT_WIDTH - BOUNDARY_PADDING;
  universe->boundary.bottom = WINDOW_DEFAULT_HEIGHT - BOUNDARY_PADDING;
  universe->boundary.enabled = true;

  return universe;
}

void UniverseDestroy(Universe *universe) {
  if (!universe)
    return;

  free(universe->entityMasks);
  free(universe->activeEntities);
  free(universe->kineticBodies);
  free(universe->mechanics);

  free(universe);
}

EntityID UniverseCreateEntity(Universe *universe) {
  if (!universe)
    return INVALID_ENTITY;

  if (universe->entityCount >= universe->maxEntities)
    return INVALID_ENTITY;

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i]) {
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

  if (!universe->activeEntities[entity])
    return false;

  universe->activeEntities[entity] = false;
  universe->entityMasks[entity] = COMPONENT_NONE;
  universe->entityCount--;
  return true;
}

bool UniverseAddKineticBodyComponent(Universe *universe, EntityID entity,
                                     KVector2 position, double mass) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  universe->kineticBodies[entity].position = position;
  universe->kineticBodies[entity].previous = position;

  if (mass <= 0 || isinf(mass)) {
    universe->kineticBodies[entity].inverseMass = 0.0;
  } else {
    universe->kineticBodies[entity].inverseMass = 1.0 / mass;
  }

  universe->entityMasks[entity] |= COMPONENT_PARTICLE;
  return true;
}

bool UniverseAddMechanicsComponent(Universe *universe, EntityID entity,
                                   KVector2 velocity,
                                   KVector2 acceleration) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  universe->mechanics[entity].velocity = velocity;
  universe->mechanics[entity].acceleration = acceleration;
  universe->mechanics[entity].forceAccum.x = 0.0;
  universe->mechanics[entity].forceAccum.y = 0.0;

  universe->entityMasks[entity] |= COMPONENT_MECHANICS;
  return true;
}

KineticBodyComponent *UniverseGetKineticBodyComponent(Universe *universe,
                                                      EntityID entity) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return NULL;

  if (!(universe->entityMasks[entity] & COMPONENT_PARTICLE))
    return NULL;

  return &universe->kineticBodies[entity];
}

MechanicsComponent *UniverseGetMechanicsComponent(Universe *universe,
                                                  EntityID entity) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return NULL;

  if (!(universe->entityMasks[entity] & COMPONENT_MECHANICS))
    return NULL;

  return &universe->mechanics[entity];
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

  universe->kineticBodies[entity].previous = position;

  return entity;
}
