#include "engine.h"

void UniverseUpdate(Universe *universe, double deltaTime) {
  if (!universe || deltaTime <= 0.0)
    return;

  int substeps = PHYSICS_SUBSTEPS;
  if (substeps < 1)
    substeps = 1;

  double stepDelta = deltaTime / (double)substeps;

  for (int step = 0; step < substeps; step++) {
    PhysicsForcesUpdate(universe);
    PhysicsMechanicsUpdate(universe, stepDelta);
    PhysicsPositionUpdate(universe, stepDelta);
    PhysicsClearForces(universe);

    UniverseUpdateSpatialGrid(universe);
    PhysicsResolveParticleCollisions(universe, stepDelta);

    if (universe->boundary.enabled)
      PhysicsResolveBoundaryCollisions(universe);
  }
}
