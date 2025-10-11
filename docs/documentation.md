# Documentation Conventions

This document outlines the documentation standards for the Kurage Physics Engine project.

## 1. Code Documentation

### 1.1 File Headers

Every source file should begin with a header comment that includes:
- Brief description of the file's purpose
- Author information
- Creation date
- License information if applicable

Example:
```c
/**
 * kurage_math.h
 * 
 * Mathematical utility functions for the Kurage Physics Engine.
 * Provides vector, matrix operations and other mathematical constructs.
 * 
 * Author: [Author Name]
 * Created: YYYY-MM-DD
 */
```

### 1.2 Function Documentation

Functions should be documented with:
- Brief description of purpose
- Parameters and their meaning
- Return value and meaning
- Any side effects or exceptions
- Example usage for complex functions

Example:
```c
/**
 * Applies a force to a particle over a time step.
 * 
 * @param particle Pointer to the particle to update
 * @param force Force vector to apply
 * @param deltaTime Time step in seconds
 * 
 * @return 0 if successful, error code otherwise
 * 
 * @note This function updates the particle's velocity and position
 */
int ApplyForce(Particle* particle, Vector2 force, double deltaTime);
```

### 1.3 Struct and Typedef Documentation

Document all fields and their purpose:

```c
/**
 * Represents a rigid body in the physics simulation.
 */
typedef struct {
  /** Position of the body's center of mass in world space */
  Vector2 position;
  
  /** Linear velocity in units per second */
  Vector2 velocity;
  
  /** Angular orientation in radians */
  double orientation;
  
  /** Inverse mass (1/mass) for efficient calculations */
  double inverseMass;
} RigidBody;
```

### 1.4 Constants and Macros

Document the purpose and any constraints:

```c
/** Maximum number of entities in the simulation */
#define MAX_ENTITIES 1000

/** Gravity acceleration in m/s² */
#define GRAVITY_CONSTANT 9.81f
```

## 2. External Documentation

### 2.1 Markdown Files

- Use consistent headers: # for title, ## for sections, ### for subsections
- Include code examples where applicable
- Use code blocks with language specification (```` ```c ````)
- Use tables for structured data
- Use bulleted lists for unordered items
- Use numbered lists for sequential steps
- Include diagrams for complex concepts (store in `/docs/images/`)

### 2.2 API Documentation

- Group related functions together
- Include examples for common use cases
- Document preconditions and postconditions
- Explain limitations and performance considerations