
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

    KMask receiverMask = universe->entityMasks[i];
    if ((receiverMask & (MASK_MECHANIC | MASK_BODY)) !=
        (MASK_MECHANIC | MASK_BODY)) {
      continue;
    }

    KMechanic *mechanic = &universe->mechanics[i];
    KBody *body = &universe->bodies[i];
    KVector2 force = KVector2Zero();

    for (uint32_t j = 0; j < universe->maxEntities; j++) {
      if (!universe->activeEntities[j]) {
        continue;
      }

      KMask sourceMask = universe->entityMasks[j];
      if ((sourceMask & (MASK_FIELD | MASK_MECHANIC | MASK_BODY)) !=
          (MASK_FIELD | MASK_MECHANIC | MASK_BODY)) {
        continue;
      }

      if (i == j) {
        continue;
      }

      KField *field = &universe->fields[j];
      KMechanic *field_mechanic = &universe->mechanics[j];
      KBody *field_body = &universe->bodies[j];

      // if (!field->apply) {
      //   continue;
      // }

      KVector2 field_force = field->apply(field_mechanic->pos, field_body->mass,
                                          mechanic->pos, body->mass);
      force = KVector2Add(force, field_force);
    }

    KVector2 acceleration = KVector2Scale(force, body->invMass);
    mechanic->acc = KVector2Add(mechanic->acc, acceleration);

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

KVector2 GravitationalField(const KVector2 origin_pos, double origin_mass,
                            const KVector2 pos, double mass) {
  const double G = 6.67 * 1e-11;
  KVector2 direction = KVector2Sub(origin_pos, pos);
  double distance_sq = direction.x * direction.x + direction.y * direction.y;

  double min_distance = 6371 * 6371 * 1e6; // prevent singularity at Earth's radius
  double force_magnitude = (G * origin_mass * mass) / (distance_sq + min_distance);
  KVector2 force = KVector2Scale(KVector2Unit(direction), force_magnitude);
  return force;
}