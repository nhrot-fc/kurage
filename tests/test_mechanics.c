#include "test_framework.h"

#include <math.h>
#include <stdio.h>

#include "../src/core/physics/mechanics.h"

static int test_mechanics_position_updates_velocity(void) {
  Universe *universe = UniverseCreate(1);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return 1;
  }

  EntityID id = UniverseCreateEntity(universe);
  if (id == INVALID_ENTITY) {
    fprintf(stderr, "Failed to create entity\n");
    UniverseDestroy(universe);
    return 1;
  }

  KMechanic mechanic = {
      .prev_pos = {42.0, 24.0},
      .pos = {1.0, -3.0},
      .vel = {2.0, 4.0},
      .acc = {0.0, 0.0},
  };

  if (!UniverseAddKMechanic(universe, id, mechanic)) {
    fprintf(stderr, "Failed to add mechanic component\n");
    UniverseDestroy(universe);
    return 1;
  }

  const double deltaTime = 0.5;
  if (!MechanicsPositionUpdate(universe, deltaTime)) {
    fprintf(stderr, "MechanicsPositionUpdate returned false\n");
    UniverseDestroy(universe);
    return 1;
  }

  const KMechanic *result = &universe->mechanics[id];
  const double epsilon = 1e-9;
  const double expected_x = mechanic.pos.x + mechanic.vel.x * deltaTime;
  const double expected_y = mechanic.pos.y + mechanic.vel.y * deltaTime;

  if (fabs(result->prev_pos.x - mechanic.pos.x) > epsilon ||
      fabs(result->prev_pos.y - mechanic.pos.y) > epsilon) {
    fprintf(stderr, "prev_pos not updated to previous position\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(result->pos.x - expected_x) > epsilon ||
      fabs(result->pos.y - expected_y) > epsilon) {
    fprintf(stderr, "Position update mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(result->vel.x - mechanic.vel.x) > epsilon ||
      fabs(result->vel.y - mechanic.vel.y) > epsilon) {
    fprintf(stderr, "Velocity should remain unchanged during position update\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

static int test_mechanics_boundary_collision_clamps_and_inverts(void) {
  Universe *universe = UniverseCreate(1);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return 1;
  }

  UniverseBoundary boundary = {
      .left = -5.0,
      .right = 5.0,
      .top = -5.0,
      .bottom = 5.0,
      .thickness = 1.0,
      .enabled = true,
  };
  if (!UniverseSetBoundary(universe, boundary)) {
    fprintf(stderr, "Failed to set boundary\n");
    UniverseDestroy(universe);
    return 1;
  }

  EntityID id = UniverseCreateEntity(universe);
  if (id == INVALID_ENTITY) {
    fprintf(stderr, "Failed to create entity\n");
    UniverseDestroy(universe);
    return 1;
  }

  KMechanic mechanic = {
      .prev_pos = {0.0, 0.0},
      .pos = {5.5, 5.5},
      .vel = {3.0, -7.0},
      .acc = {0.0, 0.0},
  };
  KParticle particle = {
      .radius = 1.5,
  };

  if (!UniverseAddKMechanic(universe, id, mechanic)) {
    fprintf(stderr, "Failed to add mechanic component\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!UniverseAddKParticle(universe, id, particle)) {
    fprintf(stderr, "Failed to add particle component\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!MechanicsBoundaryCollisionUpdate(universe)) {
    fprintf(stderr, "MechanicsBoundaryCollisionUpdate returned false\n");
    UniverseDestroy(universe);
    return 1;
  }

  const KMechanic *result = &universe->mechanics[id];
  const double epsilon = 1e-9;
  const double expected_x = boundary.right - particle.radius;
  const double expected_y = boundary.bottom - particle.radius;

  if (fabs(result->pos.x - expected_x) > epsilon ||
    fabs(result->pos.y - expected_y) > epsilon) {
    fprintf(stderr, "Positions not clamped inside boundary\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(result->vel.x + mechanic.vel.x) > epsilon ||
    fabs(result->vel.y + mechanic.vel.y) > epsilon) {
    fprintf(stderr, "Velocities not inverted on collision\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

static int test_mechanics_boundary_collision_no_change_when_inside(void) {
  Universe *universe = UniverseCreate(1);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return 1;
  }

  UniverseBoundary boundary = {
      .left = -5.0,
      .right = 5.0,
      .top = -5.0,
      .bottom = 5.0,
      .thickness = 1.0,
      .enabled = true,
  };
  if (!UniverseSetBoundary(universe, boundary)) {
    fprintf(stderr, "Failed to set boundary\n");
    UniverseDestroy(universe);
    return 1;
  }

  EntityID id = UniverseCreateEntity(universe);
  if (id == INVALID_ENTITY) {
    fprintf(stderr, "Failed to create entity\n");
    UniverseDestroy(universe);
    return 1;
  }

  KMechanic mechanic = {
      .prev_pos = {0.0, 0.0},
      .pos = {0.0, 0.0},
      .vel = {1.0, -2.0},
      .acc = {0.0, 0.0},
  };

  if (!UniverseAddKMechanic(universe, id, mechanic)) {
    fprintf(stderr, "Failed to add mechanic component\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!MechanicsBoundaryCollisionUpdate(universe)) {
    fprintf(stderr, "MechanicsBoundaryCollisionUpdate returned false\n");
    UniverseDestroy(universe);
    return 1;
  }

  const KMechanic *result = &universe->mechanics[id];
  const double epsilon = 1e-9;

  if (fabs(result->pos.x - mechanic.pos.x) > epsilon ||
      fabs(result->pos.y - mechanic.pos.y) > epsilon) {
    fprintf(stderr, "Position should remain unchanged when inside boundary\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(result->vel.x - mechanic.vel.x) > epsilon ||
      fabs(result->vel.y - mechanic.vel.y) > epsilon) {
    fprintf(stderr, "Velocity should remain unchanged when inside boundary\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

static int test_mechanics_boundary_collision_handles_top_left(void) {
  Universe *universe = UniverseCreate(1);
  if (!universe) {
    fprintf(stderr, "Failed to create universe\n");
    return 1;
  }

  UniverseBoundary boundary = {
      .left = -5.0,
      .right = 5.0,
      .top = -5.0,
      .bottom = 5.0,
      .thickness = 1.0,
      .enabled = true,
  };
  if (!UniverseSetBoundary(universe, boundary)) {
    fprintf(stderr, "Failed to set boundary\n");
    UniverseDestroy(universe);
    return 1;
  }

  EntityID id = UniverseCreateEntity(universe);
  if (id == INVALID_ENTITY) {
    fprintf(stderr, "Failed to create entity\n");
    UniverseDestroy(universe);
    return 1;
  }

  KMechanic mechanic = {
      .prev_pos = {0.0, 0.0},
      .pos = {-5.5, -5.5},
      .vel = {-9.0, 2.0},
      .acc = {0.0, 0.0},
  };

  if (!UniverseAddKMechanic(universe, id, mechanic)) {
    fprintf(stderr, "Failed to add mechanic component\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!MechanicsBoundaryCollisionUpdate(universe)) {
    fprintf(stderr, "MechanicsBoundaryCollisionUpdate returned false\n");
    UniverseDestroy(universe);
    return 1;
  }

  const KMechanic *result = &universe->mechanics[id];
  const double epsilon = 1e-9;
  const double expected_pos = boundary.left + 1.0; // default radius

  if (fabs(result->pos.x - expected_pos) > epsilon ||
      fabs(result->pos.y - expected_pos) > epsilon) {
    fprintf(stderr, "Positions not clamped to left/top boundary\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (fabs(result->vel.x + mechanic.vel.x) > epsilon ||
    fabs(result->vel.y + mechanic.vel.y) > epsilon) {
    fprintf(stderr, "Velocity inversion for left/top collision failed\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_mechanics_position_updates_velocity", test_mechanics_position_updates_velocity},
      {"test_mechanics_boundary_collision_clamps_and_inverts", test_mechanics_boundary_collision_clamps_and_inverts},
      {"test_mechanics_boundary_collision_no_change_when_inside", test_mechanics_boundary_collision_no_change_when_inside},
      {"test_mechanics_boundary_collision_handles_top_left", test_mechanics_boundary_collision_handles_top_left},
  };

  return RUN_TEST_SUITE("MECHANICS", tests);
}
