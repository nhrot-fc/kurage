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
double KVector2Norm(KVector2 v);
KVector2 KVector2Negate(KVector2 v);
KVector2 KVector2Unit(KVector2 v);
KVector2 KVector2Scale(KVector2 v, double scalar);
double KVector2DotProduct(KVector2 v1, KVector2 v2);
KVector2 KVector2Add(KVector2 v1, KVector2 v2);
KVector2 KVector2Sub(KVector2 v1, KVector2 v2);
KVector2 KVector2Zero();

/**
 * Basic KVector3 operations
 */
double KVector3Norm(KVector3 v);
KVector3 KVector3Negate(KVector3 v);
KVector3 KVector3Unit(KVector3 v);
KVector3 KVector3Scale(KVector3 v, double scalar);
double KVector3DotProduct(KVector3 v1, KVector3 v2);
KVector3 KVector3CrossProduct(KVector3 v1, KVector3 v2);
KVector3 KVector3Add(KVector3 v1, KVector3 v2);
KVector3 KVector3Sub(KVector3 v1, KVector3 v2);
KVector3 KVector3Zero();

/**
 * Basic KQuaternion operations
 */
KQuaternion KQuaternionFromAxisAngle(KVector3 axis, double angle);
KQuaternion KQuaternionMultiply(KQuaternion q1, KQuaternion q2);
Matrix3x3 KQuaternionToMatrix3(KQuaternion q);
Matrix4x4 KQuaternionToMatrix4(KQuaternion q);
KQuaternion KQuaternionUnit(KQuaternion q);

/**
 * Utility functions
 */
double KRad2Deg(double radians);
double KDeg2Rad(double degrees);
double KLerp(double a, double b, double t);
double KClamp(double value, double min, double max);

#endif /* KURAGE_MATH_H */