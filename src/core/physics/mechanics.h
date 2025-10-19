#ifndef PHYSICS_MECHANICS_H
#define PHYSICS_MECHANICS_H

#include <stdbool.h>

#include "../universe.h"

bool MechanicsBoundaryCollisionUpdate(Universe *universe);

/*
    Instant Force application
*/
bool MechanicsApplyForce(Universe *universe, EntityID id, KVector2 force);
bool MechanicsUpdate(Universe *universe, double deltaTime);
bool MechanicsCleanUp(Universe *universe);
#endif /* PHYSICS_MECHANICS_H */
