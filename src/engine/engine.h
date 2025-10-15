/**
 * kurage_engine.h
 *
 * Core entity-component-system (ECS) architecture for the Kurage Physics
 * Engine. Implements a simplified data-oriented design for basic particle
 * physics simulation.
 *
 */
#ifndef ENGINE_H
#define ENGINE_H

#include "../config/config.h"
#include "kurage_math.h"
#include <stdbool.h>
#include <stdint.h>

#define DELTA_TIME 0.1
#define GRAVITY_X 0.0
#define GRAVITY_Y 9.81
#define RESTITUTION 0.8

/*-----------------------------------------------------------------------------
 * ECS Types and Definitions
 *----------------------------------------------------------------------------*/

#define KURAGE_MAX_ENTITIES MAX_OBJECTS
typedef uint32_t EntityID;
#define INVALID_ENTITY UINT32_MAX

/**
 * Boundary structure representing the walls of the universe
 */
typedef struct {
    double left;
    double right;
    double top;
    double bottom;
    bool enabled;
} UniverseBoundary;

/**
 * Component type masks for identifying what components an entity has
 */
typedef enum {
  COMPONENT_NONE = 0,
  COMPONENT_PARTICLE =
      1 << 0, /**< Entity has particle properties (position and mass) */
  COMPONENT_MECHANICS =
      1 << 1, /**< Entity has mechanics properties (velocity and forces) */
} ComponentMask;

/**
 * Component for storing basic particle properties
 * Contains position and mass information
 */
typedef struct {
  KVector2 position;  /**< Current position */
  KVector2 previous;  /**< Previous position (for verlet integration) */
  double inverseMass; /**< Inverse of mass for efficient calculations */
} KineticBodyComponent;

/**
 * Component for storing mechanics properties
 * Contains velocity, acceleration and force accumulator
 */
typedef struct {
  KVector2 velocity;     /**< Current velocity */
  KVector2 acceleration; /**< Current acceleration */
  KVector2 forceAccum;   /**< Accumulated forces to be applied */
} MechanicsComponent;

/**
 * Universe structure that contains all entities and components
 * This is the main container for the physics simulation
 */
typedef struct {
  /** Entity management */
  uint32_t entityCount;       /**< Current number of active entities */
  uint32_t maxEntities;       /**< Maximum number of entities supported */
  ComponentMask *entityMasks; /**< Bit masks of components for each entity */
  bool *activeEntities;       /**< Whether each entity slot is active */

  /** Component storage - contiguous arrays for cache efficiency */
  KineticBodyComponent *kineticBodies;
  MechanicsComponent *mechanics;
  
  /** Universe boundaries */
  UniverseBoundary boundary;
} Universe;

/*-----------------------------------------------------------------------------
 * Universe Management Functions
 *----------------------------------------------------------------------------*/

/**
 * Creates and initializes a new universe
 *
 * @param maxEntities Maximum number of entities this universe can contain
 * @return Initialized universe instance
 */
Universe *UniverseCreate(uint32_t maxEntities);

/**
 * Destroys a universe and frees all associated resources
 *
 * @param universe Universe to destroy
 */
void UniverseDestroy(Universe *universe);

/**
 * Updates the universe state by the specified time step
 *
 * @param universe Universe to update
 * @param deltaTime Time step in seconds
 */
void UniverseUpdate(Universe *universe, double deltaTime);

/*-----------------------------------------------------------------------------
 * Entity Management Functions
 *----------------------------------------------------------------------------*/

/**
 * Creates a new entity in the universe
 *
 * @param universe Universe to create the entity in
 * @return ID of the created entity, or INVALID_ENTITY if creation failed
 */
EntityID UniverseCreateEntity(Universe *universe);

/**
 * Destroys an entity and removes all its components
 *
 * @param universe Universe containing the entity
 * @param entity ID of the entity to destroy
 * @return true if successful, false if entity didn't exist
 */
bool UniverseDestroyEntity(Universe *universe, EntityID entity);

/*-----------------------------------------------------------------------------
 * Component Management Functions
 *----------------------------------------------------------------------------*/

/**
 * Adds a kineic body component to an entity
 *
 * @param universe Universe containing the entity
 * @param entity Entity to add the component to
 * @param position Initial position value
 * @param mass Mass value (use INFINITY for static objects)
 * @return true if successful, false if failed
 */
bool UniverseAddKineticBodyComponent(Universe *universe, EntityID entity,
                                  KVector2 position, double mass);

/**
 * Adds a mechanics component to an entity
 *
 * @param universe Universe containing the entity
 * @param entity Entity to add the component to
 * @param velocity Initial velocity
 * @param acceleration Initial acceleration
 * @return true if successful, false if failed
 */
bool UniverseAddMechanicsComponent(Universe *universe, EntityID entity,
                                   KVector2 velocity, KVector2 acceleration);

/*-----------------------------------------------------------------------------
 * Component Access Functions
 *----------------------------------------------------------------------------*/

/**
 * Gets the kinetic body component of an entity
 *
 * @param universe Universe containing the entity
 * @param entity Entity to get the component from
 * @return Pointer to the component, or NULL if entity doesn't have it
 */
KineticBodyComponent *UniverseGetKineticBodyComponent(Universe *universe,
                                                EntityID entity);

/**
 * Gets the mechanics component of an entity
 *
 * @param universe Universe containing the entity
 * @param entity Entity to get the component from
 * @return Pointer to the component, or NULL if entity doesn't have it
 */
MechanicsComponent *UniverseGetMechanicsComponent(Universe *universe,
                                                  EntityID entity);

/*-----------------------------------------------------------------------------
 * Physics System Functions
 *----------------------------------------------------------------------------*/

/**
 * Applies a force to an entity
 *
 * @param universe Universe containing the entity
 * @param entity Entity to apply the force to
 * @param force Force vector to apply
 * @return true if successful, false if entity can't have forces applied
 */
bool PhysicsApplyForce(Universe *universe, EntityID entity, KVector2 force);

/**
 * Applies forces (like gravity) to all applicable entities
 *
 * @param universe Universe to process
 */
void PhysicsForcesUpdate(Universe *universe);

/**
 * Updates mechanics (acceleration and velocity) for all applicable entities
 *
 * @param universe Universe to update
 * @param deltaTime Time step in seconds
 */
void PhysicsMechanicsUpdate(Universe *universe, double deltaTime);

/**
 * Updates positions based on velocities for all applicable entities
 *
 * @param universe Universe to update
 * @param deltaTime Time step in seconds
 */
void PhysicsPositionUpdate(Universe *universe, double deltaTime);

/**
 * Clears force accumulators for the next integration step
 *
 * @param universe Universe to update
 */
void PhysicsClearForces(Universe *universe);

/**
 * Sets the universe boundaries based on window dimensions and padding
 * 
 * @param universe Universe to update boundaries for
 * @param windowWidth Width of the window in pixels
 * @param windowHeight Height of the window in pixels
 * @param padding Distance from window edge to boundary
 * @param enabled Whether boundary collision is enabled
 */
void UniverseSetBoundaries(Universe *universe, int windowWidth, int windowHeight, 
                          float padding, bool enabled);

/**
 * Resolves boundary collisions for all particles
 * 
 * @param universe Universe to process
 */
void PhysicsResolveBoundaryCollisions(Universe *universe);

/**
 * Creates a basic particle entity with position, velocity, and mass
 *
 * @param universe Universe to create the particle in
 * @param position Initial position
 * @param velocity Initial velocity
 * @param mass Mass of the particle
 * @return EntityID of the created particle
 */
EntityID ParticleCreate(Universe *universe, KVector2 position,
                        KVector2 velocity, double mass);

#endif // ENGINE_H