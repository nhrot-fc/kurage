#ifndef ECS_UNIVERSE_H
#define ECS_UNIVERSE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../config/config.h"
#include "math/kurage_math.h"
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
  uint32_t count;
  uint32_t capacity;
  EntityID *entities;
} GridCell;

typedef struct {
  double cellSize;
  uint32_t columns;
  uint32_t rows;
  GridCell *cells;
} Grid;

typedef struct {
  uint32_t entityCount;
  uint32_t maxEntities;
  ComponentMask *entityMasks;
  bool *activeEntities;
  ParticleComponent *particles;
  KineticBodyComponent *kineticBodies;
  MechanicsComponent *mechanics;
  UniverseBoundary boundary;
  Grid grid;
} Universe;

Universe *UniverseCreate(uint32_t maxEntities);
void UniverseDestroy(Universe *universe);
EntityID UniverseCreateEntity(Universe *universe);
bool UniverseDestroyEntity(Universe *universe, EntityID entity);
bool UniverseAddParticleComponent(Universe *universe, EntityID entity,
                                  double radius, double density);
bool UniverseAddKineticBodyComponent(Universe *universe, EntityID entity,
                                     KVector2 position, double mass);
bool UniverseAddMechanicsComponent(Universe *universe, EntityID entity,
                                   KVector2 velocity);
ParticleComponent *UniverseGetParticleComponent(Universe *universe,
                                                EntityID entity);
KineticBodyComponent *UniverseGetKineticBodyComponent(Universe *universe,
                                                      EntityID entity);
MechanicsComponent *UniverseGetMechanicsComponent(Universe *universe,
                                                  EntityID entity);
void UniverseSetBoundaries(Universe *universe, int windowWidth,
                           int windowHeight, float padding, bool enabled);
void UniverseUpdateSpatialGrid(Universe *universe);
size_t UniverseQueryNeighbors(const Universe *universe, KVector2 position,
                              double radius, EntityID *outEntities,
                              size_t maxEntities);
void UniverseInformation(const Universe *universe);
#endif /* ECS_UNIVERSE_H */
