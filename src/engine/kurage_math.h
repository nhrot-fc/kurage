/**
 * kurage_math.h
 *
 * A basic mathematics library for the Kurage Physic
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
} KVector3;

typedef struct {
  double x;
  double y;
} KVector2;

/**
 * KQuaternion structure
 * Used for representing rotations in 3D space
 */
typedef struct {
  double x; // X component
  double y; // Y component
  double z; // Z component
  double w; // Real component
} KQuaternion;

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
 * Basic KVector2 operations
 */
double KVector2Magnitude(KVector2 *v);
KVector2 KVector2Inversion(KVector2 *v);
KVector2 KVector2Normalization(KVector2 *v);
KVector2 KVector2ScalarProduct(double scalar, KVector2 *v);
double KVector2DotProduct(KVector2 *v1, KVector2 *v2);
KVector2 KVector2Addition(KVector2 *v1, KVector2 *v2);
KVector2 KVector2Subtraction(KVector2 *v1, KVector2 *v2);

/**
 * Basic KVector3 operations
 */
double KVector3Magnitude(KVector3 *v);
KVector3 KVector3Inversion(KVector3 *v);
KVector3 KVector3Normalization(KVector3 *v);
KVector3 KVector3ScalarProduct(double scalar, KVector3 *v);
double KVector3DotProduct(KVector3 *v1, KVector3 *v2);
KVector3 KVector3CrossProduct(KVector3 *v1, KVector3 *v2);
KVector3 KVector3Addition(KVector3 *v1, KVector3 *v2);
KVector3 KVector3Subtraction(KVector3 *v1, KVector3 *v2);

/**
 * Basic KQuaternion operations
 */
KQuaternion KQuaternionFromAxisAngle(KVector3 *axis, double angle);
KQuaternion KQuaternionMultiply(KQuaternion *q1, KQuaternion *q2);
Matrix3x3 KQuaternionToMatrix3(KQuaternion *q);
Matrix4x4 KQuaternionToMatrix4(KQuaternion *q);
KQuaternion KQuaternionNormalize(KQuaternion *q);

/**
 * Utility functions
 */
double RadiansToDegrees(double radians);
double DegreesToRadians(double degrees);
double Lerp(double a, double b, double t);
double Clamp(double value, double min, double max);

#endif /* KURAGE_MATH_H */