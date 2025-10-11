# Kurage Physics Engine Conventions

This document outlines the coding and documentation conventions for the Kurage Physics Engine project.

## 1. Naming Conventions

### 1.1 File Naming

- Header files use the `.h` extension
- Implementation files use the `.c` extension
- Files should be named using lowercase letters with underscores separating words
- Common prefixes:
  - `kurage_` - Core engine components
  - `test_` - Test files

Examples:
```
kurage_math.h
kurage_math.c
```

### 1.2 Type Naming

- Struct and typedef names use PascalCase
- Enum names use PascalCase
- Enum members use ALL_CAPS with underscores

Examples:
```c
typedef struct {
  double x;
  double y;
} Vector2;

typedef enum {
  COMPONENT_POSITION,
  COMPONENT_VELOCITY,
  COMPONENT_ACCELERATION
} ComponentType;
```

### 1.3 Function Naming

- Functions use PascalCase for public API
- Internal/private functions use camelCase
- Functions should follow a `Noun_Verb` or `NounVerb` pattern

Examples:
```c
// Public API
Vector2 Vector2Add(Vector2 a, Vector2 b);

// Internal function
static void updateParticlePosition(Particle* particle, double deltaTime);
```

### 1.4 Variable Naming

- Local variables use camelCase
- Global variables use g_camelCase (with g_ prefix)
- Constants use ALL_CAPS with underscores

Examples:
```c
// Local variable
double deltaTime = 0.016;

// Global variable
Vector2 g_gravity = {0.0, -9.81};

// Constant
#define MAX_ENTITIES 1000
```

### 1.5 Component Naming

In the ECS architecture:
- Component types should end with `Component`
- System names should end with `System`

Examples:
```c
typedef struct {
  Vector2 position;
  Vector2 previous;
} PositionComponent;

void MovementSystem_Update(World* world, double deltaTime);
```

## 2. Code Organization

- Each component should be defined in its own header file when possible
- Implementation details should be hidden in source files
- Public API should be clean and well-documented in headers
- Related components and systems should be grouped in logical subdirectories

## 3. Memory Management

- Always initialize memory before use
- Always free allocated memory
- Use defensive programming (null checks, bounds checking)
- Consider using a memory pool for frequently allocated/freed objects