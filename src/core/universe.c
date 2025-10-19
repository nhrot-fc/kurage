#include "universe.h"

#include <stdlib.h>

Universe *UniverseCreate(uint32_t maxEntities) {
  Universe *universe = (Universe *)malloc(sizeof(Universe));
  if (!universe) {
    return NULL;
  }

  universe->entityCount = 0;
  universe->maxEntities = maxEntities;
  universe->nextEntityId = 0;
  universe->freeEntityCount = 0;

  universe->entityMasks = (KMask *)calloc(maxEntities, sizeof(KMask));
  universe->activeEntities = (bool *)calloc(maxEntities, sizeof(bool));
  universe->mechanics = (KMechanic *)calloc(maxEntities, sizeof(KMechanic));
  universe->bodies = (KBody *)calloc(maxEntities, sizeof(KBody));
  universe->particles = (KParticle *)calloc(maxEntities, sizeof(KParticle));
  universe->fields = (KField *)calloc(maxEntities, sizeof(KField));
  universe->freeEntityStack = (EntityID *)calloc(maxEntities, sizeof(EntityID));

  if (!universe->entityMasks || !universe->activeEntities ||
      !universe->mechanics || !universe->bodies || !universe->particles ||
      !universe->fields || !universe->freeEntityStack) {
    UniverseDestroy(universe);
    return NULL;
  }

  for (uint32_t i = 0; i < maxEntities; i++) {
    universe->entityMasks[i] = MASK_NONE;
    universe->activeEntities[i] = false;
  }

  universe->boundary.enabled = false;

  return universe;
}

bool UniverseDestroy(Universe *universe) {
  if (universe == NULL) {
    return false;
  }
  free(universe->entityMasks);
  free(universe->activeEntities);
  free(universe->mechanics);
  free(universe->bodies);
  free(universe->particles);
  free(universe->fields);
  free(universe->freeEntityStack);
  universe->entityMasks = NULL;
  universe->activeEntities = NULL;
  universe->mechanics = NULL;
  universe->bodies = NULL;
  universe->particles = NULL;
  universe->fields = NULL;
  universe->freeEntityStack = NULL;
  free(universe);
  return true;
}

bool UniverseSetBoundary(Universe *universe, UniverseBoundary boundary) {
  if (universe == NULL) {
    return false;
  }
  universe->boundary.enabled = true;
  universe->boundary = boundary;
  return true;
}

bool UniverseUnsetBoundary(Universe *universe) {
  if (universe == NULL) {
    return false;
  }
  universe->boundary.enabled = false;
  return true;
}

EntityID UniverseCreateEntity(Universe *universe) {
  if (universe == NULL) {
    return INVALID_ENTITY;
  }
  if (universe->entityCount >= universe->maxEntities) {
    return INVALID_ENTITY;
  }

  EntityID newId;
  if (universe->freeEntityCount > 0) {
    newId = universe->freeEntityStack[--universe->freeEntityCount];
  } else {
    if (universe->nextEntityId >= universe->maxEntities) {
      return INVALID_ENTITY;
    }
    newId = universe->nextEntityId++;
  }

  universe->entityCount++;
  universe->activeEntities[newId] = true;
  universe->entityMasks[newId] = MASK_NONE;
  universe->mechanics[newId] = (KMechanic){0};
  universe->bodies[newId] = (KBody){0};
  universe->particles[newId] = (KParticle){0};
  universe->fields[newId] = (KField){0};
  return newId;
}

bool UniverseDestroyEntity(Universe *universe, EntityID id) {
  if (!UniverseIsEntityActive(universe, id)) {
    return false;
  }
  universe->activeEntities[id] = false;
  universe->entityMasks[id] = MASK_NONE;

  universe->mechanics[id] = (KMechanic){0};
  universe->bodies[id] = (KBody){0};
  universe->particles[id] = (KParticle){0};
  universe->fields[id] = (KField){0};
  universe->entityCount--;
  if (universe->freeEntityCount < universe->maxEntities) {
    universe->freeEntityStack[universe->freeEntityCount++] = id;
  }
  return true;
}

bool UniverseIsEntityActive(const Universe *universe, EntityID id) {
  return universe != NULL && id < universe->maxEntities &&
         universe->activeEntities[id];
}

#define DEFINE_COMPONENT_ACCESSORS(Name, Field, Mask, Type)                    \
  bool UniverseAdd##Name(Universe *universe, EntityID id, Type component) {    \
    if (!UniverseIsEntityActive(universe, id)) {                               \
      return false;                                                            \
    }                                                                          \
    universe->Field[id] = component;                                           \
    universe->entityMasks[id] |= (Mask);                                       \
    return true;                                                               \
  }                                                                            \
  bool UniverseRemove##Name(Universe *universe, EntityID id) {                 \
    if (!UniverseIsEntityActive(universe, id)) {                               \
      return false;                                                            \
    }                                                                          \
    universe->Field[id] = (Type){0};                                           \
    universe->entityMasks[id] &= ~(Mask);                                      \
    return true;                                                               \
  }

DEFINE_COMPONENT_ACCESSORS(KMechanic, mechanics, MASK_MECHANIC, KMechanic)
DEFINE_COMPONENT_ACCESSORS(KBody, bodies, MASK_BODY, KBody)
DEFINE_COMPONENT_ACCESSORS(KParticle, particles, MASK_PARTICLE, KParticle)
DEFINE_COMPONENT_ACCESSORS(KField, fields, MASK_FIELD, KField)

#undef DEFINE_COMPONENT_ACCESSORS