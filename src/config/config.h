#ifndef CONFIG_H
#define CONFIG_H

/*
 * Global configuration values for the Kurage engine. These defaults describe
 * the simulation limits and rendering hints and can be overridden by future
 * runtime configuration systems.
 */

/* Entity/Simulation limits */
#define MAX_OBJECTS 100
#define OBJECT_RADIUS 5

/* Physics parameters */
#define GRAVITY_X 0.0
#define GRAVITY_Y 9.81
#define RESTITUTION 0.8
#define DEFAULT_MASS 1.0

/* Boundary handling */
#define BOUNDARY_PADDING 10.0f

/* Window defaults (used when the renderer cannot query current size) */
#define WINDOW_DEFAULT_WIDTH 800
#define WINDOW_DEFAULT_HEIGHT 600

#endif /* CONFIG_H */
