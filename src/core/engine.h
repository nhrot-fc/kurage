/**
 * Central header that aggregates the engine-facing API.
 */
#ifndef ENGINE_H
#define ENGINE_H

#include "universe.h"
#include "physics/systems.h"

void UniverseUpdate(Universe *universe, double deltaTime);

#endif /* ENGINE_H */