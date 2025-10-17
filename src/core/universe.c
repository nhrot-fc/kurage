#include "universe.h"

#include <math.h>
#include <stdlib.h>

#define UNIVERSE_DEFAULT_CELL_SIZE (30.0)

static uint32_t GridIndex(const Grid *grid, uint32_t column, uint32_t row) {
  return row * grid->columns + column;
}

static void GridFree(Grid *grid) {
  if (!grid || !grid->cells)
    return;

  uint32_t total = grid->columns * grid->rows;
  for (uint32_t i = 0; i < total; i++) {
    free(grid->cells[i].entities);
    grid->cells[i].entities = NULL;
    grid->cells[i].capacity = 0;
    grid->cells[i].count = 0;
  }

  free(grid->cells);
  grid->cells = NULL;
  grid->columns = 0;
  grid->rows = 0;
}

static bool GridAllocate(const UniverseBoundary *boundary, double cellSize,
                         Grid *outGrid) {
  if (!outGrid || cellSize <= 0.0)
    return false;

  double width = boundary->right - boundary->left;
  double height = boundary->bottom - boundary->top;

  if (width <= 0.0)
    width = cellSize;
  if (height <= 0.0)
    height = cellSize;

  uint32_t columns = (uint32_t)ceil(width / cellSize);
  uint32_t rows = (uint32_t)ceil(height / cellSize);

  if (columns == 0)
    columns = 1;
  if (rows == 0)
    rows = 1;

  Grid temp = {0};
  temp.cellSize = cellSize;
  temp.columns = columns;
  temp.rows = rows;
  temp.cells = (GridCell *)calloc(columns * rows, sizeof(GridCell));
  if (!temp.cells)
    return false;

  *outGrid = temp;
  return true;
}

static bool GridResize(Grid *grid, const UniverseBoundary *boundary) {
  if (!grid)
    return false;

  Grid newGrid = {0};
  newGrid.cellSize = grid->cellSize;

  if (!GridAllocate(boundary, newGrid.cellSize, &newGrid))
    return false;

  GridFree(grid);
  *grid = newGrid;
  return true;
}

static void GridClear(Grid *grid) {
  if (!grid || !grid->cells)
    return;

  uint32_t total = grid->columns * grid->rows;
  for (uint32_t i = 0; i < total; i++) {
    grid->cells[i].count = 0;
  }
}

static bool GridEnsureCapacity(GridCell *cell, uint32_t required) {
  if (cell->capacity >= required)
    return true;

  uint32_t newCapacity = cell->capacity ? cell->capacity : 4;
  while (newCapacity < required)
    newCapacity *= 2;

  EntityID *entities =
      (EntityID *)realloc(cell->entities, newCapacity * sizeof(EntityID));
  if (!entities)
    return false;

  cell->entities = entities;
  cell->capacity = newCapacity;
  return true;
}

static bool UniverseComputeCell(const Universe *universe, KVector2 position,
                                uint32_t *cellX, uint32_t *cellY) {
  if (!universe || !universe->grid.cells || universe->grid.columns == 0 ||
      universe->grid.rows == 0)
    return false;

  double left = universe->boundary.left;
  double right = universe->boundary.right;
  double top = universe->boundary.top;
  double bottom = universe->boundary.bottom;

  if (right <= left || bottom <= top)
    return false;

  double x = position.x;
  double y = position.y;

  if (x <= left)
    x = left;
  else if (x >= right)
    x = right - 1e-6;

  if (y <= top)
    y = top;
  else if (y >= bottom)
    y = bottom - 1e-6;

  double dx = x - left;
  double dy = y - top;

  uint32_t column = (uint32_t)(dx / universe->grid.cellSize);
  uint32_t row = (uint32_t)(dy / universe->grid.cellSize);

  if (column >= universe->grid.columns)
    column = universe->grid.columns - 1;
  if (row >= universe->grid.rows)
    row = universe->grid.rows - 1;

  if (cellX)
    *cellX = column;
  if (cellY)
    *cellY = row;
  return true;
}

Universe *UniverseCreate(uint32_t maxEntities) {
  Universe *universe = (Universe *)malloc(sizeof(Universe));
  if (!universe)
    return NULL;

  universe->entityCount = 0;
  universe->maxEntities = maxEntities;

  universe->grid.cellSize = UNIVERSE_DEFAULT_CELL_SIZE;
  universe->grid.columns = 0;
  universe->grid.rows = 0;
  universe->grid.cells = NULL;

  universe->entityMasks = NULL;
  universe->activeEntities = NULL;
  universe->particles = NULL;
  universe->kineticBodies = NULL;
  universe->mechanics = NULL;

  universe->entityMasks =
      (ComponentMask *)calloc(maxEntities, sizeof(ComponentMask));
  universe->activeEntities = (bool *)calloc(maxEntities, sizeof(bool));
  universe->particles =
      (ParticleComponent *)calloc(maxEntities, sizeof(ParticleComponent));
  universe->kineticBodies =
      (KineticBodyComponent *)calloc(maxEntities, sizeof(KineticBodyComponent));
  universe->mechanics =
      (MechanicsComponent *)calloc(maxEntities, sizeof(MechanicsComponent));

  if (!universe->entityMasks || !universe->activeEntities ||
      !universe->particles || !universe->kineticBodies ||
      !universe->mechanics) {
    UniverseDestroy(universe);
    return NULL;
  }

  universe->boundary.left = BOUNDARY_PADDING;
  universe->boundary.top = BOUNDARY_PADDING;
  universe->boundary.right = WINDOW_DEFAULT_WIDTH - BOUNDARY_PADDING;
  universe->boundary.bottom = WINDOW_DEFAULT_HEIGHT - BOUNDARY_PADDING;
  universe->boundary.enabled = true;

  if (!GridAllocate(&universe->boundary, universe->grid.cellSize,
                    &universe->grid)) {
    UniverseDestroy(universe);
    return NULL;
  }

  return universe;
}

void UniverseDestroy(Universe *universe) {
  if (!universe)
    return;

  GridFree(&universe->grid);

  free(universe->entityMasks);
  free(universe->activeEntities);
  free(universe->particles);
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
  universe->particles[entity] = (ParticleComponent){0};
  universe->entityCount--;
  return true;
}

bool UniverseAddParticleComponent(Universe *universe, EntityID entity,
                                  double radius, double density) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return false;

  ParticleComponent component = {0};
  component.radius = radius;
  component.density = (density > 0.0) ? density : 1.0;

  universe->particles[entity] = component;
  universe->entityMasks[entity] |= COMPONENT_PARTICLE;
  return true;
}

ParticleComponent *UniverseGetParticleComponent(Universe *universe,
                                                EntityID entity) {
  if (!universe || entity >= universe->maxEntities ||
      !universe->activeEntities[entity])
    return NULL;

  if (!(universe->entityMasks[entity] & COMPONENT_PARTICLE))
    return NULL;

  return &universe->particles[entity];
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

  universe->entityMasks[entity] |= COMPONENT_KINETIC;
  return true;
}

bool UniverseAddMechanicsComponent(Universe *universe, EntityID entity,
                                   KVector2 velocity, KVector2 acceleration) {
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

  if (!(universe->entityMasks[entity] & COMPONENT_KINETIC))
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

  if (!GridResize(&universe->grid, &universe->boundary)) {
    UniverseUpdateSpatialGrid(universe);
    return;
  }
  UniverseUpdateSpatialGrid(universe);
}

void UniverseUpdateSpatialGrid(Universe *universe) {
  if (!universe || !universe->grid.cells)
    return;

  GridClear(&universe->grid);

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    ComponentMask required = COMPONENT_KINETIC | COMPONENT_PARTICLE;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    uint32_t cellX = 0;
    uint32_t cellY = 0;
    if (!UniverseComputeCell(universe, universe->kineticBodies[i].position,
                             &cellX, &cellY))
      continue;

    GridCell *cell =
        &universe->grid.cells[GridIndex(&universe->grid, cellX, cellY)];
    if (!GridEnsureCapacity(cell, cell->count + 1))
      continue;

    cell->entities[cell->count++] = i;
  }
}

bool UniverseGetCellCoords(const Universe *universe, KVector2 position,
                           uint32_t *cellX, uint32_t *cellY) {
  return UniverseComputeCell(universe, position, cellX, cellY);
}

size_t UniverseGetCellNeighbors(const Universe *universe, uint32_t cellX,
                                uint32_t cellY, EntityID *outEntities,
                                size_t maxEntities) {
  if (!universe || !universe->grid.cells || universe->grid.columns == 0 ||
      universe->grid.rows == 0 || !outEntities || maxEntities == 0)
    return 0;

  if (cellX >= universe->grid.columns || cellY >= universe->grid.rows)
    return 0;

  size_t written = 0;
  uint32_t minX = (cellX == 0) ? 0 : cellX - 1;
  uint32_t minY = (cellY == 0) ? 0 : cellY - 1;
  uint32_t maxX = (cellX + 1 >= universe->grid.columns)
                      ? universe->grid.columns - 1
                      : cellX + 1;
  uint32_t maxY =
      (cellY + 1 >= universe->grid.rows) ? universe->grid.rows - 1 : cellY + 1;

  for (uint32_t y = minY; y <= maxY && written < maxEntities; y++) {
    for (uint32_t x = minX; x <= maxX && written < maxEntities; x++) {
      const GridCell *cell =
          &universe->grid.cells[GridIndex(&universe->grid, x, y)];
      for (uint32_t i = 0; i < cell->count && written < maxEntities; i++) {
        outEntities[written++] = cell->entities[i];
      }
    }
  }

  return written;
}

size_t UniverseQueryNeighbors(const Universe *universe, KVector2 position,
                              double radius, EntityID *outEntities,
                              size_t maxEntities) {
  if (!universe || !outEntities || maxEntities == 0)
    return 0;

  if (!universe->grid.cells || universe->grid.columns == 0 ||
      universe->grid.rows == 0)
    return 0;

  uint32_t centerX = 0;
  uint32_t centerY = 0;
  if (!UniverseComputeCell(universe, position, &centerX, &centerY))
    return 0;

  double searchRadius = (radius > 0.0) ? radius : 0.0;
  double cellSize = universe->grid.cellSize;
  uint32_t range = (uint32_t)ceil(searchRadius / cellSize);

  uint32_t minX = (centerX > range) ? centerX - range : 0;
  uint32_t minY = (centerY > range) ? centerY - range : 0;
  uint32_t maxX = centerX + range;
  uint32_t maxY = centerY + range;
  if (maxX >= universe->grid.columns)
    maxX = universe->grid.columns - 1;
  if (maxY >= universe->grid.rows)
    maxY = universe->grid.rows - 1;

  size_t written = 0;
  for (uint32_t y = minY; y <= maxY && written < maxEntities; y++) {
    for (uint32_t x = minX; x <= maxX && written < maxEntities; x++) {
      const GridCell *cell =
          &universe->grid.cells[GridIndex(&universe->grid, x, y)];
      for (uint32_t i = 0; i < cell->count && written < maxEntities; i++) {
        EntityID candidate = cell->entities[i];
        if (candidate >= universe->maxEntities)
          continue;
        ComponentMask mask = universe->entityMasks[candidate];
        ComponentMask required = COMPONENT_KINETIC | COMPONENT_PARTICLE;
        if ((mask & required) != required)
          continue;

        const KineticBodyComponent *body = &universe->kineticBodies[candidate];
        double dx = body->position.x - position.x;
        double dy = body->position.y - position.y;
        double distanceSq = dx * dx + dy * dy;
        double neighborRadius = universe->particles[candidate].radius;
        double combinedRadius = searchRadius + neighborRadius;
        if (combinedRadius < 0.0)
          combinedRadius = 0.0;

        if (distanceSq <= combinedRadius * combinedRadius)
          outEntities[written++] = candidate;
      }
    }
  }

  return written;
}
