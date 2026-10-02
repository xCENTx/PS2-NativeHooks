#ifndef SOCOM_MATH_H
#define SOCOM_MATH_H

#include "structs.h"

Vec3 Vec3_Add(Vec3 a, Vec3 b);
Vec3 QuaternionRotate(Vec4 q, Vec3 v);
Vec3 TransformPoint(const Matrix4x4* matrix, Vec3 point);
Vec4 TransformPoint4(const Matrix4x4* matrix, Vec4 point);

f32 FastAbs(f32 value);
f32 FastLength2D(f32 x, f32 y);

Matrix4x4 MatrixMultiply(Matrix4x4 a, Matrix4x4 b);

#endif