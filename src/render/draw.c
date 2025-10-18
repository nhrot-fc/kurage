#include "draw.h"

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
    KMask required = MASK_MECHANIC | MASK_PARTICLE;
    if (!universe->activeEntities[i] ||
        (universe->entityMasks[i] & required) != required)
      continue;

    const KMechanic *kineticBody = &universe->mechanics[i];
    double radius = universe->particles[i].radius;
    Color particleColor = WHITE;

    float velocityVectorLength = 0.0f;
    Vector2 velocityDir = {0.0f, 0.0f};
    if (universe->entityMasks[i] & MASK_MECHANIC) {
      const KMechanic *mechanics = &universe->mechanics[i];
      double speed = KVector2Norm(mechanics->vel);
      particleColor = color_for_speed(speed);

      if (speed > 1e-4) {
        KVector2 kvelDir = KVector2Unit(mechanics->vel);
        velocityDir.x = (float)kvelDir.x;
        velocityDir.y = (float)kvelDir.y;

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

    Vector2 center = {(float)kineticBody->pos.x, (float)kineticBody->pos.y};
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

void RenderUniverseGrid(const Universe *universe) {}
