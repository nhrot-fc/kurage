#ifndef ECS_COMPONENTS_H
#define ECS_COMPONENTS_H

#include <stdbool.h>
#include <stdint.h>

#include "math/kurage_math.h"

typedef enum {
	COMPONENT_NONE = 0,
	COMPONENT_KINETIC = 1 << 0,
	COMPONENT_MECHANICS = 1 << 1,
	COMPONENT_PARTICLE = 1 << 2,
} ComponentMask;

typedef struct {
	KVector2 position;
	KVector2 previous;
	double inverseMass;
} KineticBodyComponent;

typedef struct {
	double radius;
	double density;
} ParticleComponent;

typedef struct {
	KVector2 velocity;
	KVector2 forceAccum;
	KVector2 constantForces;
	KVector2 acceleration;
	bool needsVerletSync;
} MechanicsComponent;

#endif /* ECS_COMPONENTS_H */
