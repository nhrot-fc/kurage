#include "draw.h"

#include <math.h>

#include "../../lib/raylib/src/raylib.h"

static const double SPEED_COLOR_MAX = 160.0;
static const float SPEED_COLOR_SATURATION = 0.85f;
static const float SPEED_COLOR_VALUE = 0.95f;
static const float VELOCITY_VECTOR_MIN_LENGTH = 6.0f;
static const float VELOCITY_VECTOR_MAX_LENGTH = 32.0f;

static Color color_for_speed(double speed) {
  double normalized = speed / SPEED_COLOR_MAX;
  if (normalized < 0.0)
    normalized = 0.0;
  if (normalized > 1.0)
    normalized = 1.0;

  float hue = (float)((1.0 - normalized) * 220.0); // Cyan -> Red ramp.
  return ColorFromHSV(hue, SPEED_COLOR_SATURATION, SPEED_COLOR_VALUE);
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

    float velocityVectorLength = 0.0f;
    Vector2 velocityDir = {0.0f, 0.0f};
    if (universe->entityMasks[i] & COMPONENT_MECHANICS) {
      const MechanicsComponent *mechanics = &universe->mechanics[i];
      double speed = sqrt(mechanics->velocity.x * mechanics->velocity.x +
                          mechanics->velocity.y * mechanics->velocity.y);
      particleColor = color_for_speed(speed);

      if (speed > 1e-4) {
        double invSpeed = 1.0 / speed;
        velocityDir.x = (float)(mechanics->velocity.x * invSpeed);
        velocityDir.y = (float)(mechanics->velocity.y * invSpeed);

        double scaledLength = speed / SPEED_COLOR_MAX;
        if (scaledLength < 0.0)
          scaledLength = 0.0;
        if (scaledLength > 1.0)
          scaledLength = 1.0;

        velocityVectorLength =
            VELOCITY_VECTOR_MIN_LENGTH +
            (float)scaledLength *
                (VELOCITY_VECTOR_MAX_LENGTH - VELOCITY_VECTOR_MIN_LENGTH);
      }
    }

    Vector2 center = {(float)kineticBody->position.x,
                      (float)kineticBody->position.y};
    float radiusF = (float)radius;

    DrawCircleV(center, radiusF, ColorAlpha(particleColor, 0.85f));
    DrawCircleLines(center.x, center.y, radiusF,
                   ColorAlpha(particleColor, 0.35f));

    if (velocityVectorLength > 0.0f) {
      Vector2 end = {center.x + velocityDir.x * velocityVectorLength,
                     center.y + velocityDir.y * velocityVectorLength};
      DrawLineEx(center, end, 2.0f, ColorAlpha(particleColor, 0.7f));
    }
  }
}

void RenderUniverseGrid(const Universe *universe) {
  if (!universe || !universe->grid.cells || universe->grid.columns == 0 ||
      universe->grid.rows == 0)
    return;

  const Grid *grid = &universe->grid;

  Color outlineColor = ColorAlpha(SKYBLUE, 0.5f);
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
      if (alpha > 0.9)
        alpha = 0.9;

      if (alpha > 0.0) {
        Color fillColor = ColorAlpha(SKYBLUE, alpha);
        DrawRectangle((int)floor(x0), (int)floor(y0), width, height, fillColor);
      }
      DrawRectangleLines((int)floor(x0), (int)floor(y0), width, height,
                         outlineColor);
    }
  }
}
