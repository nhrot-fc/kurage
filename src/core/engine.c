#include "engine.h"

void UniverseUpdate(Universe *universe, double deltaTime) {
  if (!universe || deltaTime <= 0.0) {
    return;
  }

  MechanicsPositionUpdate(universe, deltaTime);
  MechanicsBoundaryCollisionUpdate(universe);
}
