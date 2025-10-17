#include "engine.h"

void UniverseUpdate(Universe *universe, double deltaTime) {
  if (!universe)
    return;

  PhysicsForcesUpdate(universe);
  PhysicsMechanicsUpdate(universe, deltaTime);
  PhysicsPositionUpdate(universe, deltaTime);
  PhysicsClearForces(universe);

  UniverseUpdateSpatialGrid(universe);
  PhysicsResolveParticleCollisions(universe);

  if (universe->boundary.enabled)
    PhysicsResolveBoundaryCollisions(universe);

  UniverseUpdateSpatialGrid(universe);
}
