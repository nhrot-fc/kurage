#ifndef ECS_UNIVERSE_H
#define ECS_UNIVERSE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../config/config.h"
#include "components.h"

typedef uint32_t EntityID;
#define INVALID_ENTITY UINT32_MAX
typedef struct {
  double left;
  double right;
  double top;
  double bottom;
  double thickness;
  bool enabled;
} UniverseBoundary;

typedef struct {
  uint32_t entityCount;
  uint32_t maxEntities;
  uint32_t nextEntityId;
  uint32_t freeEntityCount;
  KMask *entityMasks;
  bool *activeEntities;
  KMechanic *mechanics;
  KBody *bodies;
  KParticle *particles;
  KField *fields;
  EntityID *freeEntityStack;
  UniverseBoundary boundary;
} Universe;

Universe *UniverseCreate(uint32_t maxEntities);
bool UniverseDestroy(Universe *universe);
bool UniverseSetBoundary(Universe *universe, UniverseBoundary boundary);
bool UniverseUnsetBoundary(Universe *universe);

EntityID UniverseCreateEntity(Universe *universe);
bool UniverseDestroyEntity(Universe *universe, EntityID id);
bool UniverseIsEntityActive(const Universe *universe, EntityID id);

#define DECLARE_COMPONENT_ACCESSORS(Name, Type)                                \
  bool UniverseAdd##Name(Universe *universe, EntityID id, Type component);     \
  bool UniverseRemove##Name(Universe *universe, EntityID id)

DECLARE_COMPONENT_ACCESSORS(KMechanic, KMechanic);
DECLARE_COMPONENT_ACCESSORS(KBody, KBody);
DECLARE_COMPONENT_ACCESSORS(KParticle, KParticle);
DECLARE_COMPONENT_ACCESSORS(KField, KField);

#undef DECLARE_COMPONENT_ACCESSORS

#endif /* ECS_UNIVERSE_H */
