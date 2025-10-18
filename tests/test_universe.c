#include "test_framework.h"

#include <stdio.h>
#include <string.h>

#include "../src/core/universe.h"

static int test_universe_create(void) {
  const uint32_t maxEntities = 8;
  Universe *universe = UniverseCreate(maxEntities);

  if (universe->maxEntities != maxEntities) {
    fprintf(stderr, "maxEntities mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (universe->entityCount != 0) {
    fprintf(stderr, "entityCount should start at zero\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (universe->nextEntityId != 0) {
    fprintf(stderr, "nextEntityId should start at zero\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (universe->freeEntityCount != 0) {
    fprintf(stderr, "freeEntityCount should start at zero\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (universe->boundary.enabled) {
    fprintf(stderr, "boundary should be disabled by default\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (!universe->entityMasks || !universe->activeEntities ||
      !universe->mechanics || !universe->bodies || !universe->particles ||
      !universe->freeEntityStack) {
    fprintf(stderr, "UniverseCreate should allocate component arrays\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

static int test_universe_entity_life(void) {
  Universe *universe = UniverseCreate(2);

  EntityID first = UniverseCreateEntity(universe);
  if (first == INVALID_ENTITY) {
    fprintf(stderr, "Expected valid entity id for first entity\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (!universe->activeEntities[first] || universe->entityCount != 1) {
    fprintf(stderr, "First entity not properly registered\n");
    UniverseDestroy(universe);
    return 1;
  }

  EntityID second = UniverseCreateEntity(universe);
  if (second == INVALID_ENTITY || universe->entityCount != 2) {
    fprintf(stderr, "Second entity creation failed\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (UniverseCreateEntity(universe) != INVALID_ENTITY) {
    fprintf(stderr, "Creation beyond capacity should fail\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!UniverseDestroyEntity(universe, first)) {
    fprintf(stderr, "Destroying first entity should succeed\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (universe->activeEntities[first] || universe->entityCount != 1 ||
      universe->freeEntityCount != 1) {
    fprintf(stderr, "Destroy logic did not release first entity\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

static int test_universe_reuses_destroyed_id(void) {
  Universe *universe = UniverseCreate(3);

  EntityID a = UniverseCreateEntity(universe);
  EntityID b = UniverseCreateEntity(universe);
  EntityID c = UniverseCreateEntity(universe);
  if (a == INVALID_ENTITY || b == INVALID_ENTITY || c == INVALID_ENTITY) {
    fprintf(stderr, "Failed to create initial entities\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!UniverseDestroyEntity(universe, b)) {
    fprintf(stderr, "Unable to destroy middle entity\n");
    UniverseDestroy(universe);
    return 1;
  }

  EntityID reused = UniverseCreateEntity(universe);
  if (reused != b) {
    fprintf(stderr, "Expected to reuse freed entity id\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (universe->freeEntityCount != 0 || universe->entityCount != 3) {
    fprintf(stderr, "Post-reuse state mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

static int test_universe_add_remove_mechanic(void) {
  Universe *universe = UniverseCreate(1);
  EntityID id = UniverseCreateEntity(universe);
  KMechanic mechanic = {
      .prev_pos = {1.0, 2.0},
      .pos = {3.0, 4.0},
      .vel = {5.0, 6.0},
      .acc = {7.0, 8.0},
  };

  if (!UniverseAddKMechanic(universe, id, mechanic)) {
    fprintf(stderr, "Failed to add mechanic component\n");
    UniverseDestroy(universe);
    return 1;
  }
  if ((universe->entityMasks[id] & MASK_MECHANIC) == 0) {
    fprintf(stderr, "Mechanic mask flag missing\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (memcmp(&universe->mechanics[id], &mechanic, sizeof(KMechanic)) != 0) {
    fprintf(stderr, "Mechanic data mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!UniverseRemoveKMechanic(universe, id)) {
    fprintf(stderr, "Failed to remove mechanic component\n");
    UniverseDestroy(universe);
    return 1;
  }
  if ((universe->entityMasks[id] & MASK_MECHANIC) != 0) {
    fprintf(stderr, "Mechanic mask flag should be cleared\n");
    UniverseDestroy(universe);
    return 1;
  }

  const KMechanic zeroMechanic = {0};
  if (memcmp(&universe->mechanics[id], &zeroMechanic, sizeof(KMechanic)) != 0) {
    fprintf(stderr, "Mechanic data should be zeroed on removal\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

static int test_universe_add_remove_body(void) {
  Universe *universe = UniverseCreate(1);
  EntityID id = UniverseCreateEntity(universe);
  KBody body = {
      .invMass = 0.25,
      .mass = 4.0,
  };

  if (!UniverseAddKBody(universe, id, body)) {
    fprintf(stderr, "Failed to add body component\n");
    UniverseDestroy(universe);
    return 1;
  }
  if ((universe->entityMasks[id] & MASK_BODY) == 0) {
    fprintf(stderr, "Body mask flag missing\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (memcmp(&universe->bodies[id], &body, sizeof(KBody)) != 0) {
    fprintf(stderr, "Body data mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!UniverseRemoveKBody(universe, id)) {
    fprintf(stderr, "Failed to remove body component\n");
    UniverseDestroy(universe);
    return 1;
  }
  if ((universe->entityMasks[id] & MASK_BODY) != 0) {
    fprintf(stderr, "Body mask flag should be cleared\n");
    UniverseDestroy(universe);
    return 1;
  }

  const KBody zeroBody = {0};
  if (memcmp(&universe->bodies[id], &zeroBody, sizeof(KBody)) != 0) {
    fprintf(stderr, "Body data should be zeroed on removal\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

static int test_universe_add_remove_particle(void) {
  Universe *universe = UniverseCreate(1);
  EntityID id = UniverseCreateEntity(universe);
  KParticle particle = {
      .radius = 2.5,
  };

  if (!UniverseAddKParticle(universe, id, particle)) {
    fprintf(stderr, "Failed to add particle component\n");
    UniverseDestroy(universe);
    return 1;
  }
  if ((universe->entityMasks[id] & MASK_PARTICLE) == 0) {
    fprintf(stderr, "Particle mask flag missing\n");
    UniverseDestroy(universe);
    return 1;
  }
  if (memcmp(&universe->particles[id], &particle, sizeof(KParticle)) != 0) {
    fprintf(stderr, "Particle data mismatch\n");
    UniverseDestroy(universe);
    return 1;
  }

  if (!UniverseRemoveKParticle(universe, id)) {
    fprintf(stderr, "Failed to remove particle component\n");
    UniverseDestroy(universe);
    return 1;
  }
  if ((universe->entityMasks[id] & MASK_PARTICLE) != 0) {
    fprintf(stderr, "Particle mask flag should be cleared\n");
    UniverseDestroy(universe);
    return 1;
  }

  const KParticle zeroParticle = {0};
  if (memcmp(&universe->particles[id], &zeroParticle, sizeof(KParticle)) != 0) {
    fprintf(stderr, "Particle data should be zeroed on removal\n");
    UniverseDestroy(universe);
    return 1;
  }

  UniverseDestroy(universe);
  return 0;
}

int main(void) {
  const TestCase tests[] = {
      {"test_universe_create", test_universe_create},
      {"test_universe_entity_life", test_universe_entity_life},
      {"test_universe_reuses_destroyed_id", test_universe_reuses_destroyed_id},
      {"test_universe_add_remove_mechanic", test_universe_add_remove_mechanic},
      {"test_universe_add_remove_body", test_universe_add_remove_body},
      {"test_universe_add_remove_particle", test_universe_add_remove_particle},
  };

  return RUN_TEST_SUITE("UNIVERSE", tests);
}
