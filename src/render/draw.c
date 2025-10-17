#include "draw.h"

#include <math.h>

#include "../../lib/raylib/src/raylib.h"

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
    ComponentMask required = COMPONENT_KINETIC | COMPONENT_PARTICLE;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    const KineticBodyComponent *kineticBody = &universe->kineticBodies[i];
    double radius = universe->particles[i].radius;
    Color particleColor = WHITE;

    if (universe->entityMasks[i] & COMPONENT_MECHANICS) {
      const MechanicsComponent *mechanics = &universe->mechanics[i];
      double speed = sqrt(mechanics->velocity.x * mechanics->velocity.x +
                          mechanics->velocity.y * mechanics->velocity.y);
      particleColor = color_for_speed(speed);
    }

    DrawCircle((int)kineticBody->position.x, (int)kineticBody->position.y,
               (float)radius, particleColor);
  }
}

void RenderUniverseGrid(const Universe *universe) {
  if (!universe || !universe->grid.cells || universe->grid.columns == 0 ||
      universe->grid.rows == 0)
    return;

  const Grid *grid = &universe->grid;

  Color outlineColor = ColorAlpha(SKYBLUE, 0.4f);
  double baseX = universe->boundary.left;
  double baseY = universe->boundary.top;
  double right = universe->boundary.right;
  double bottom = universe->boundary.bottom;

  for (uint32_t row = 0; row < grid->rows; row++) {
    for (uint32_t col = 0; col < grid->columns; col++) {
      const GridCell *cell = &grid->cells[row * grid->columns + col];
      double x0 = baseX + col * grid->cellSize;
      double y0 = baseY + row * grid->cellSize;
      double x1 = fmin(x0 + grid->cellSize, right);
      double y1 = fmin(y0 + grid->cellSize, bottom);
      int width = (int)ceil(x1 - x0);
      int height = (int)ceil(y1 - y0);
      if (width <= 0 || height <= 0)
        continue;

      double alpha = cell->count * 0.25;
      if (alpha > 1.0)
        alpha = 1.0;

      if (alpha > 0.0) {
        Color fillColor = ColorAlpha(SKYBLUE, alpha);
        DrawRectangle((int)floor(x0), (int)floor(y0), width, height, fillColor);
      }
      DrawRectangleLines((int)floor(x0), (int)floor(y0), width, height,
                         outlineColor);
    }
  }
}
