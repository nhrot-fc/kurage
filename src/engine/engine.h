#ifndef ENGINE_H
#define ENGINE_H

typedef struct {
  float x;
  float y;
} Vector2;

typedef struct {
  Vector2 position;
} PositionComponent;

typedef struct {
  Vector2 velocity;
  Vector2 force_accumulator;
  float mass;
} PhysicsComponent;

typedef struct {
  float density;
  float pressure;
} FluidParticleComponent;

typedef struct {
  int entity_count;
  int capacity;

  unsigned int *entity_masks;
  PositionComponent *positions;
  PhysicsComponent *physics;
  FluidParticleComponent *fluid_particles;

} Universe;

Universe *create_world(int capacity);
void destroy_world(Universe *world);
int create_particle(Universe *world, Vector2 pos, float mass);

void Gravity_update(Universe *world);
void Fluid_update(Universe *world);
void Movement_update(Universe *world, float dt);
void ForceCleanup_update(Universe *world);

#endif // ENGINE_H