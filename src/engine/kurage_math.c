/**
 * kurage_math.c
 *
 * Implementation of the Kurage Physics Engine math library functions.
 *
 */

#include "kurage_math.h"
#include <math.h>
#include <stddef.h>

/**
 * Vector2 Operations Implementation
 */
double Vector2Magnitude(Vector2 *v) {
  if (v == NULL)
    return 0.0;
  return sqrt(v->x * v->x + v->y * v->y);
}

Vector2 Vector2Inversion(Vector2 *v) {
  Vector2 result = {0, 0};
  if (v == NULL)
    return result;

  result.x = -v->x;
  result.y = -v->y;

  return result;
}

Vector2 Vector2Normalization(Vector2 *v) {
  Vector2 result = {0, 0};
  if (v == NULL)
    return result;

  double magnitude = Vector2Magnitude(v);
  if (magnitude < 0.000001) {
    return result;
  }

  result.x = v->x / magnitude;
  result.y = v->y / magnitude;

  return result;
}

Vector2 Vector2ScalarProduct(double scalar, Vector2 *v) {
  Vector2 result = {0, 0};
  if (v == NULL)
    return result;

  result.x = v->x * scalar;
  result.y = v->y * scalar;

  return result;
}

double Vector2DotProduct(Vector2 *v1, Vector2 *v2) {
  if (v1 == NULL || v2 == NULL)
    return 0.0;

  return v1->x * v2->x + v1->y * v2->y;
}

Vector2 Vector2Addition(Vector2 *v1, Vector2 *v2) {
  Vector2 result = {0, 0};
  if (v1 == NULL || v2 == NULL)
    return result;

  result.x = v1->x + v2->x;
  result.y = v1->y + v2->y;

  return result;
}

Vector2 Vector2Subtraction(Vector2 *v1, Vector2 *v2) {
  Vector2 result = {0, 0};
  if (v1 == NULL || v2 == NULL)
    return result;

  result.x = v1->x - v2->x;
  result.y = v1->y - v2->y;

  return result;
}

/**
 * Vector3 Operations Implementation
 */

double Vector3Magnitude(Vector3 *v) {
  if (v == NULL)
    return 0.0;
  return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

Vector3 Vector3Inversion(Vector3 *v) {
  Vector3 result = {0, 0, 0};
  if (v == NULL)
    return result;

  result.x = -v->x;
  result.y = -v->y;
  result.z = -v->z;

  return result;
}

Vector3 Vector3Normalization(Vector3 *v) {
  Vector3 result = {0, 0, 0};
  if (v == NULL)
    return result;

  double magnitude = Vector3Magnitude(v);
  if (magnitude < 0.000001) {
    return result;
  }

  result.x = v->x / magnitude;
  result.y = v->y / magnitude;
  result.z = v->z / magnitude;

  return result;
}

Vector3 Vector3ScalarProduct(double scalar, Vector3 *v) {
  Vector3 result = {0, 0, 0};
  if (v == NULL)
    return result;

  result.x = v->x * scalar;
  result.y = v->y * scalar;
  result.z = v->z * scalar;

  return result;
}

double Vector3DotProduct(Vector3 *v1, Vector3 *v2) {
  if (v1 == NULL || v2 == NULL)
    return 0.0;

  return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

Vector3 Vector3CrossProduct(Vector3 *v1, Vector3 *v2) {
  Vector3 result = {0, 0, 0};
  if (v1 == NULL || v2 == NULL)
    return result;

  result.x = v1->y * v2->z - v1->z * v2->y;
  result.y = v1->z * v2->x - v1->x * v2->z;
  result.z = v1->x * v2->y - v1->y * v2->x;

  return result;
}

Vector3 Vector3Addition(Vector3 *v1, Vector3 *v2) {
  Vector3 result = {0, 0, 0};
  if (v1 == NULL || v2 == NULL)
    return result;

  result.x = v1->x + v2->x;
  result.y = v1->y + v2->y;
  result.z = v1->z + v2->z;

  return result;
}

Vector3 Vector3Subtraction(Vector3 *v1, Vector3 *v2) {
  Vector3 result = {0, 0, 0};
  if (v1 == NULL || v2 == NULL)
    return result;

  result.x = v1->x - v2->x;
  result.y = v1->y - v2->y;
  result.z = v1->z - v2->z;

  return result;
}

/**
 * Quaternion Operations Implementation
 */

Quaternion QuaternionFromAxisAngle(Vector3 *axis, double angle) {
  Quaternion result = {0, 0, 0, 1}; // Default to identity quaternion
  if (axis == NULL)
    return result;

  Vector3 normalized = Vector3Normalization(axis);
  double halfAngle = angle / 2.0;
  double sinHalfAngle = sin(halfAngle);

  result.x = normalized.x * sinHalfAngle;
  result.y = normalized.y * sinHalfAngle;
  result.z = normalized.z * sinHalfAngle;
  result.w = cos(halfAngle);

  return result;
}

Quaternion QuaternionMultiply(Quaternion *q1, Quaternion *q2) {
  Quaternion result = {0, 0, 0, 1}; // Default to identity quaternion
  if (q1 == NULL || q2 == NULL)
    return result;

  result.w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;
  result.x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
  result.y = q1->w * q2->y - q1->x * q2->z + q1->y * q2->w + q1->z * q2->x;
  result.z = q1->w * q2->z + q1->x * q2->y - q1->y * q2->x + q1->z * q2->w;

  return result;
}

Matrix3x3 QuaternionToMatrix3(Quaternion *q) {
  Matrix3x3 result = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}}; // Identity matrix
  if (q == NULL)
    return result;

  double xx = q->x * q->x;
  double yy = q->y * q->y;
  double zz = q->z * q->z;
  double xy = q->x * q->y;
  double xz = q->x * q->z;
  double yz = q->y * q->z;
  double wx = q->w * q->x;
  double wy = q->w * q->y;
  double wz = q->w * q->z;

  result.m[0][0] = 1.0 - 2.0 * (yy + zz);
  result.m[0][1] = 2.0 * (xy - wz);
  result.m[0][2] = 2.0 * (xz + wy);

  result.m[1][0] = 2.0 * (xy + wz);
  result.m[1][1] = 1.0 - 2.0 * (xx + zz);
  result.m[1][2] = 2.0 * (yz - wx);

  result.m[2][0] = 2.0 * (xz - wy);
  result.m[2][1] = 2.0 * (yz + wx);
  result.m[2][2] = 1.0 - 2.0 * (xx + yy);

  return result;
}

Matrix4x4 QuaternionToMatrix4(Quaternion *q) {
  // Initialize identity matrix
  Matrix4x4 result = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};

  if (q == NULL)
    return result;

  double xx = q->x * q->x;
  double yy = q->y * q->y;
  double zz = q->z * q->z;
  double xy = q->x * q->y;
  double xz = q->x * q->z;
  double yz = q->y * q->z;
  double wx = q->w * q->x;
  double wy = q->w * q->y;
  double wz = q->w * q->z;

  result.m[0][0] = 1.0 - 2.0 * (yy + zz);
  result.m[0][1] = 2.0 * (xy - wz);
  result.m[0][2] = 2.0 * (xz + wy);

  result.m[1][0] = 2.0 * (xy + wz);
  result.m[1][1] = 1.0 - 2.0 * (xx + zz);
  result.m[1][2] = 2.0 * (yz - wx);

  result.m[2][0] = 2.0 * (xz - wy);
  result.m[2][1] = 2.0 * (yz + wx);
  result.m[2][2] = 1.0 - 2.0 * (xx + yy);

  return result;
}

Quaternion QuaternionNormalize(Quaternion *q) {
  Quaternion result = {0, 0, 0, 1}; // Default to identity quaternion
  if (q == NULL)
    return result;

  double magnitude =
      sqrt(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
  if (magnitude < 0.000001) { // Avoid division by zero
    return result;
  }

  result.x = q->x / magnitude;
  result.y = q->y / magnitude;
  result.z = q->z / magnitude;
  result.w = q->w / magnitude;

  return result;
}

/**
 * Utility Functions Implementation
 */
double RadiansToDegrees(double radians) { return radians * (180.0 / M_PI); }

double DegreesToRadians(double degrees) { return degrees * (M_PI / 180.0); }

double Lerp(double a, double b, double t) {
  if (t < 0.0)
    t = 0.0;
  if (t > 1.0)
    t = 1.0;

  return a + t * (b - a);
}

double Clamp(double value, double min, double max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}