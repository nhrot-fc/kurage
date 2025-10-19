
#include "mechanics.h"

bool MechanicsBoundaryCollisionUpdate(Universe *universe) {
  if (!universe) {
    return false;
  }

  if (!universe->boundary.enabled) {
    return true;
  }

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i]) {
      continue;
    }

    if (!(universe->entityMasks[i] & MASK_MECHANIC)) {
      continue;
    }

    KMechanic *mechanic = &universe->mechanics[i];
    double radius = 1.0;
    if ((universe->entityMasks[i] & MASK_PARTICLE)) {
      KParticle *particle = &universe->particles[i];
      radius = particle->radius;
    }

    double left_most = mechanic->pos.x - radius;
    double right_most = mechanic->pos.x + radius;
    double top_most = mechanic->pos.y - radius;
    double bottom_most = mechanic->pos.y + radius;

    bool collided_x = false;
    if (left_most < universe->boundary.left) {
      mechanic->pos.x = universe->boundary.left + radius;
      collided_x = true;
    } else if (right_most > universe->boundary.right) {
      mechanic->pos.x = universe->boundary.right - radius;
      collided_x = true;
    }

    bool collided_y = false;
    if (top_most < universe->boundary.top) {
      mechanic->pos.y = universe->boundary.top + radius;
      collided_y = true;
    } else if (bottom_most > universe->boundary.bottom) {
      mechanic->pos.y = universe->boundary.bottom - radius;
      collided_y = true;
    }

    if (collided_x) {
      mechanic->vel.x = -mechanic->vel.x;
    }
    if (collided_y) {
      mechanic->vel.y = -mechanic->vel.y;
    }
  }

  return true;
}

bool MechanicsApplyForce(Universe *universe, EntityID id, KVector2 force) {
  if (!universe || !UniverseIsEntityActive(universe, id)) {
    return false;
  }

  if (!(universe->entityMasks[id] & MASK_MECHANIC) ||
      !(universe->entityMasks[id] & MASK_BODY)) {
    return false;
  }

  KBody *body = &universe->bodies[id];
  KMechanic *mechanic = &universe->mechanics[id];
  KVector2 acceleration = KVector2Scale(force, body->invMass);
  mechanic->acc = KVector2Add(mechanic->acc, acceleration);

  return true;
}

bool MechanicsUpdate(Universe *universe, double deltaTime) {
  if (!universe) {
    return false;
  }

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i]) {
      continue;
    }

    if (!(universe->entityMasks[i] & MASK_MECHANIC)) {
      continue;
    }

    KMechanic *mechanic = &universe->mechanics[i];
    mechanic->vel =
        KVector2Add(mechanic->vel, KVector2Scale(mechanic->acc, deltaTime));
    mechanic->prev_pos = mechanic->pos;
    mechanic->pos =
        KVector2Add(mechanic->pos, KVector2Scale(mechanic->vel, deltaTime));
  }

  return true;
}

bool MechanicsCleanUp(Universe *universe) {
  if (!universe) {
    return false;
  }

  for (uint32_t i = 0; i < universe->maxEntities; i++) {
    if (!universe->activeEntities[i]) {
      continue;
    }

    if (!(universe->entityMasks[i] & MASK_MECHANIC)) {
      continue;
    }

    KMechanic *mechanic = &universe->mechanics[i];
    mechanic->acc = KVector2Zero();
  }

  return true;
}