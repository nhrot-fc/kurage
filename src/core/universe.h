#ifndef ECS_UNIVERSE_H
#define ECS_UNIVERSE_H

#include <stdbool.h>
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
  bool enabled;
} UniverseBoundary;

typedef struct {
  uint32_t entityCount;
  uint32_t maxEntities;
  ComponentMask *entityMasks;
  bool *activeEntities;
  KineticBodyComponent *kineticBodies;
  MechanicsComponent *mechanics;
  UniverseBoundary boundary;
} Universe;

Universe *UniverseCreate(uint32_t maxEntities);
void UniverseDestroy(Universe *universe);
EntityID UniverseCreateEntity(Universe *universe);
bool UniverseDestroyEntity(Universe *universe, EntityID entity);
bool UniverseAddKineticBodyComponent(Universe *universe, EntityID entity,
                                     KVector2 position, double mass);
bool UniverseAddMechanicsComponent(Universe *universe, EntityID entity,
                                   KVector2 velocity, KVector2 acceleration);
KineticBodyComponent *UniverseGetKineticBodyComponent(Universe *universe,
                                                      EntityID entity);
MechanicsComponent *UniverseGetMechanicsComponent(Universe *universe,
                                                  EntityID entity);
void UniverseSetBoundaries(Universe *universe, int windowWidth,
                           int windowHeight, float padding, bool enabled);
EntityID ParticleCreate(Universe *universe, KVector2 position,
                        KVector2 velocity, double mass);

#endif /* ECS_UNIVERSE_H */
