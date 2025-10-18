#ifndef PHYSICS_MECHANICS_H
#define PHYSICS_MECHANICS_H

#include <stdbool.h>

#include "../universe.h"

bool MechanicsPositionUpdate(Universe *universe, double deltaTime);
bool MechanicsBoundaryCollisionUpdate(Universe *universe);

#endif /* PHYSICS_MECHANICS_H */
