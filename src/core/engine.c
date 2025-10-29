#include "engine.h"

void UniverseUpdate(Universe *universe, double deltaTime) {
  if (!universe || deltaTime <= 0.0) {
    return;
  }
  double substeps = 8.0;
  double dt = deltaTime / substeps;
  for (int i = 0; i < (int)substeps; i++) {
    MechanicsUpdate(universe, dt);
    MechanicsBoundaryCollisionUpdate(universe);
    MechanicsCleanUp(universe);
  }
}
