#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../src/engine/engine.h"

#define EPSILON 0.001

int test_verlet_integration() {
    // Create a universe
    Universe *universe = UniverseCreate(100);
    if (!universe) {
        fprintf(stderr, "Failed to create universe\n");
        return 1;
    }

    // Create a particle with initial position and velocity
    KVector2 initial_pos = {100.0, 100.0};
    KVector2 initial_vel = {10.0, 0.0};
    double mass = 1.0;
    
    EntityID particle = ParticleCreate(universe, initial_pos, initial_vel, mass);
    if (particle == INVALID_ENTITY) {
        fprintf(stderr, "Failed to create particle\n");
        UniverseDestroy(universe);
        return 1;
    }

    // Disable boundaries for this test
    universe->boundary.enabled = false;

    // Get initial state
    KineticBodyComponent *body = UniverseGetKineticBodyComponent(universe, particle);
    MechanicsComponent *mech = UniverseGetMechanicsComponent(universe, particle);
    
    printf("Initial position: (%.2f, %.2f)\n", body->position.x, body->position.y);
    printf("Initial velocity: (%.2f, %.2f)\n", mech->velocity.x, mech->velocity.y);
    
    // The previous position is already set correctly by ParticleCreate for Verlet integration
    
    // Apply no gravity for this test - just let it move with initial velocity
    // Do one integration step
    PhysicsIntegrateForces(universe, DELTA_TIME);
    
    printf("After 1 step position: (%.2f, %.2f)\n", body->position.x, body->position.y);
    printf("After 1 step velocity: (%.2f, %.2f)\n", mech->velocity.x, mech->velocity.y);
    
    // With Verlet integration and no forces:
    // - Position should advance by velocity * dt
    // - Velocity should remain constant (derived from position difference)
    double expected_x = initial_pos.x + initial_vel.x * DELTA_TIME;
    double expected_y = initial_pos.y + initial_vel.y * DELTA_TIME;
    
    if (fabs(body->position.x - expected_x) > EPSILON || 
        fabs(body->position.y - expected_y) > EPSILON) {
        fprintf(stderr, "Position mismatch: expected (%.2f, %.2f), got (%.2f, %.2f)\n",
                expected_x, expected_y, body->position.x, body->position.y);
        UniverseDestroy(universe);
        return 1;
    }
    
    // Velocity should be approximately the same (within tolerance)
    if (fabs(mech->velocity.x - initial_vel.x) > EPSILON || 
        fabs(mech->velocity.y - initial_vel.y) > EPSILON) {
        fprintf(stderr, "Velocity mismatch: expected (%.2f, %.2f), got (%.2f, %.2f)\n",
                initial_vel.x, initial_vel.y, mech->velocity.x, mech->velocity.y);
        UniverseDestroy(universe);
        return 1;
    }

    UniverseDestroy(universe);
    printf("Verlet integration test: PASSED\n");
    return 0;
}

int test_verlet_with_constant_force() {
    // Create a universe
    Universe *universe = UniverseCreate(100);
    if (!universe) {
        fprintf(stderr, "Failed to create universe\n");
        return 1;
    }

    // Create a particle at rest
    KVector2 initial_pos = {0.0, 0.0};
    KVector2 initial_vel = {0.0, 0.0};
    double mass = 1.0;
    
    EntityID particle = ParticleCreate(universe, initial_pos, initial_vel, mass);
    if (particle == INVALID_ENTITY) {
        fprintf(stderr, "Failed to create particle\n");
        UniverseDestroy(universe);
        return 1;
    }

    // Disable boundaries
    universe->boundary.enabled = false;

    // Apply a constant force (e.g., 10 N in x direction)
    KVector2 force = {10.0, 0.0};
    PhysicsApplyForce(universe, particle, force);

    // Get state
    KineticBodyComponent *body = UniverseGetKineticBodyComponent(universe, particle);
    
    // Do one integration step
    PhysicsIntegrateForces(universe, DELTA_TIME);
    
    // With constant force F=10, mass=1, acceleration a=10
    // For Verlet from rest: new_pos = 2*current - prev + a*dt*dt
    // Since current = prev initially: new_pos = current + a*dt*dt = 0 + 10*0.1*0.1 = 0.1
    double expected_x = 10.0 * DELTA_TIME * DELTA_TIME;
    double expected_y = 0.0;
    
    printf("After force application:\n");
    printf("  Position: (%.6f, %.6f)\n", body->position.x, body->position.y);
    printf("  Expected: (%.6f, %.6f)\n", expected_x, expected_y);
    
    if (fabs(body->position.x - expected_x) > EPSILON) {
        fprintf(stderr, "Position mismatch with constant force\n");
        UniverseDestroy(universe);
        return 1;
    }

    UniverseDestroy(universe);
    printf("Verlet with constant force test: PASSED\n");
    return 0;
}

int main(void) {
    int result = 0;
    
    result |= test_verlet_integration();
    result |= test_verlet_with_constant_force();
    
    if (result == 0) {
        printf("\nAll Verlet integration tests passed!\n");
    } else {
        printf("\nSome tests failed!\n");
    }
    
    return result;
}
