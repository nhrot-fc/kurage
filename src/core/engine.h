/**
 * Central header that aggregates the engine-facing API.
 */
#ifndef ENGINE_H
#define ENGINE_H

#include "physics/mechanics.h"

void UniverseUpdate(Universe *universe, double deltaTime);

#endif /* ENGINE_H */