#ifndef PHYSICS_SYSTEMS_H
#define PHYSICS_SYSTEMS_H

#include <stdbool.h>

#include "../universe.h"

bool PhysicsApplyForce(Universe *universe, EntityID entity, KVector2 force);
void PhysicsForcesUpdate(Universe *universe);
void PhysicsMechanicsUpdate(Universe *universe, double deltaTime);
void PhysicsPositionUpdate(Universe *universe, double deltaTime);
void PhysicsClearForces(Universe *universe);
void PhysicsResolveParticleCollisions(Universe *universe, double deltaTime);
void PhysicsResolveBoundaryCollisions(Universe *universe);

#endif /* PHYSICS_SYSTEMS_H */
