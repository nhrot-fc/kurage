/**
 * kurage_math.h
 *
 * A basic mathematics library for the Kurage Physics Engine.
 * This header defines vector structures and operations commonly used
 * in physics simulations and game development.
 *
 */

#ifndef KURAGE_MATH_H
#define KURAGE_MATH_H

#include <math.h>
#include <stddef.h>

typedef struct {
  double x;
  double y;
  double z;
} Vector3;

typedef struct {
  double x;
  double y;
} Vector2;

/**
 * Quaternion structure
 * Used for representing rotations in 3D space
 */
typedef struct {
  double x; // X component
  double y; // Y component
  double z; // Z component
  double w; // Real component
} Quaternion;

/**
 * Matrix4x4 structure
 * Used for transformations in 3D space
 */
typedef struct {
  double m[4][4];
} Matrix4x4;

/**
 * Matrix3x3 structure
 * Used for transformations in 2D space and for representing inertia tensors
 */
typedef struct {
  double m[3][3];
} Matrix3x3;

/**
 * Basic Vector2 operations
 */
double Vector2Magnitude(Vector2 *v);
Vector2 Vector2Inversion(Vector2 *v);
Vector2 Vector2Normalization(Vector2 *v);
Vector2 Vector2ScalarProduct(double scalar, Vector2 *v);
double Vector2DotProduct(Vector2 *v1, Vector2 *v2);
Vector2 Vector2Addition(Vector2 *v1, Vector2 *v2);
Vector2 Vector2Subtraction(Vector2 *v1, Vector2 *v2);

/**
 * Basic Vector3 operations
 */
double Vector3Magnitude(Vector3 *v);
Vector3 Vector3Inversion(Vector3 *v);
Vector3 Vector3Normalization(Vector3 *v);
Vector3 Vector3ScalarProduct(double scalar, Vector3 *v);
double Vector3DotProduct(Vector3 *v1, Vector3 *v2);
Vector3 Vector3CrossProduct(Vector3 *v1, Vector3 *v2);
Vector3 Vector3Addition(Vector3 *v1, Vector3 *v2);
Vector3 Vector3Subtraction(Vector3 *v1, Vector3 *v2);

/**
 * Basic Quaternion operations
 */
Quaternion QuaternionFromAxisAngle(Vector3 *axis, double angle);
Quaternion QuaternionMultiply(Quaternion *q1, Quaternion *q2);
Matrix3x3 QuaternionToMatrix3(Quaternion *q);
Matrix4x4 QuaternionToMatrix4(Quaternion *q);
Quaternion QuaternionNormalize(Quaternion *q);

/**
 * Utility functions
 */
double RadiansToDegrees(double radians);
double DegreesToRadians(double degrees);
double Lerp(double a, double b, double t);
double Clamp(double value, double min, double max);

#endif /* KURAGE_MATH_H */