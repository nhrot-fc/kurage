#include "draw.h"

#include <math.h>

#include "../../lib/raylib/src/raylib.h"
#include "../config/config.h"

static Color color_for_speed(double speed) {
	if (speed > 100.0)
		return RED;
	if (speed > 50.0)
		return ORANGE;
	if (speed > 20.0)
		return YELLOW;
	if (speed > 10.0)
		return GREEN;
	return BLUE;
}

void RenderUniverse(const Universe *universe) {
	if (!universe)
		return;

	if (universe->boundary.enabled) {
		Color boundaryColor = ColorAlpha(WHITE, 0.8f);
		DrawRectangleLines(
				(int)universe->boundary.left, (int)universe->boundary.top,
				(int)(universe->boundary.right - universe->boundary.left),
				(int)(universe->boundary.bottom - universe->boundary.top),
				boundaryColor);
	}

	for (uint32_t i = 0; i < universe->maxEntities; i++) {
		if (!universe->activeEntities[i] ||
				!(universe->entityMasks[i] & COMPONENT_PARTICLE))
			continue;

		const KineticBodyComponent *particle = &universe->kineticBodies[i];
		Color particleColor = WHITE;

		if (universe->entityMasks[i] & COMPONENT_MECHANICS) {
			const MechanicsComponent *mechanics = &universe->mechanics[i];
			double speed = sqrt(mechanics->velocity.x * mechanics->velocity.x +
													mechanics->velocity.y * mechanics->velocity.y);
			particleColor = color_for_speed(speed);
		}

		DrawCircle((int)particle->position.x, (int)particle->position.y,
							 OBJECT_RADIUS, particleColor);
	}
}
