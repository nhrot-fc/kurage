#ifndef ECS_COMPONENTS_H
#define ECS_COMPONENTS_H

#include <stdbool.h>
#include <stdint.h>

#include "math/kurage_math.h"

typedef enum {
  MASK_NONE = 0,
  MASK_MECHANIC = 1 << 0,
  MASK_BODY = 1 << 1,
  MASK_PARTICLE = 1 << 2,
} KMask;

/*
  Kurage Mechanics Component
  To describe a movable entity by the classics laws of motion
*/
typedef struct {
  KVector2 prev_pos;
  KVector2 pos;
  KVector2 vel;
  KVector2 acc;
} KMechanic;

/*
  Kurage Body Component
  To describe the properties of an tangible entity with mass
  We store invMass to describe infinitely massive objects
  NOTE: Default shape is circle or sphere with radius of 1
*/
typedef struct {
  double invMass;
  double mass;
} KBody;

typedef struct {
  double radius;
} KParticle;

#endif /* ECS_COMPONENTS_H */
