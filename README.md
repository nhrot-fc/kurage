# Kurage Physics Engine

A lightweight physics engine implemented in C, focused on particle-based physics simulation using an Entity Component System architecture.

## Overview

Kurage Physics Engine is designed to provide a simple, efficient, and flexible foundation for physics simulations. It implements an Entity Component System (ECS) architecture for optimal performance and composability.

## Features

- Entity Component System architecture
- Particle physics simulation
- Vector and matrix math operations
- Force accumulation and integration
- Verlet integration for stable simulations

## Project Structure

```
/kurage
├── build/              # Compiled binaries and object files
├── docs/               # Documentation
│   ├── conventions.md  # Coding conventions
│   ├── documentation.md # Documentation standards
│   ├── ecs_architecture.md # ECS design explanation
│   └── testing.md      # Testing guidelines
├── lib/                # Third-party libraries
│   └── raylib/         # RayLib for graphics rendering
├── references/         # Reference materials
├── src/                # Source code
│   ├── config/         # Configuration
│   ├── engine/         # Core engine code
│   │   ├── engine.c    # Engine implementation
│   │   ├── engine.h    # Engine interface
│   │   ├── kurage_math.c # Math library implementation
│   │   └── kurage_math.h # Math library interface
│   └── plugin/         # Plugin system
└── tests/              # Test code
```

## Building the Project

To build the project, use the provided Makefile:

```bash
make
```

To run the application:

```bash
./build/bin/kurage
```

## Documentation

For more detailed information, see the documentation in the `docs` directory:

- [Coding Conventions](docs/conventions.md)
- [Documentation Standards](docs/documentation.md)
- [ECS Architecture](docs/ecs_architecture.md)
- [Testing Guidelines](docs/testing.md)

## Contributing

Contributions are welcome! Please read the [coding conventions](docs/conventions.md) before submitting code.

## License

[MIT License](LICENSE)