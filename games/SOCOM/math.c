#include "math.h"

Vec3 Vec3_Add(Vec3 a, Vec3 b)
{
    Vec3 out =
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };

    return out;
}

Vec3 QuaternionRotate(Vec4 q, Vec3 v)
{
    // q.xyz = imaginary component
    // q.w   = real component

    Vec3 qv =
    {
        q.x,
        q.y,
        q.z
    };

    Vec3 uv =
    {
        qv.y * v.z - qv.z * v.y,
        qv.z * v.x - qv.x * v.z,
        qv.x * v.y - qv.y * v.x
    };

    Vec3 uuv =
    {
        qv.y * uv.z - qv.z * uv.y,
        qv.z * uv.x - qv.x * uv.z,
        qv.x * uv.y - qv.y * uv.x
    };

    float s = 2.0f * q.w;

    Vec3 out =
    {
        v.x + (uv.x * s) + (uuv.x * 2.0f),
        v.y + (uv.y * s) + (uuv.y * 2.0f),
        v.z + (uv.z * s) + (uuv.z * 2.0f)
    };

    return out;
}

Vec3 TransformPoint(const Matrix4x4* m, Vec3 p)
{
    Vec3 out;

    // Matrix convention based on your translation being m[3][0..2]
    out.x =
        p.x * m->m[0][0] +
        p.y * m->m[1][0] +
        p.z * m->m[2][0] +
              m->m[3][0];

    out.y =
        p.x * m->m[0][1] +
        p.y * m->m[1][1] +
        p.z * m->m[2][1] +
              m->m[3][1];

    out.z =
        p.x * m->m[0][2] +
        p.y * m->m[1][2] +
        p.z * m->m[2][2] +
              m->m[3][2];

    return out;
}

Vec4 TransformPoint4(const Matrix4x4* m, Vec4 v)
{
    Vec4 out;
    out.x =
        v.x * m->m[0][0] +
        v.y * m->m[1][0] +
        v.z * m->m[2][0] +
        v.w * m->m[3][0];

    out.y =
        v.x * m->m[0][1] +
        v.y * m->m[1][1] +
        v.z * m->m[2][1] +
        v.w * m->m[3][1];

    out.z =
        v.x * m->m[0][2] +
        v.y * m->m[1][2] +
        v.z * m->m[2][2] +
        v.w * m->m[3][2];

    out.w =
        v.x * m->m[0][3] +
        v.y * m->m[1][3] +
        v.z * m->m[2][3] +
        v.w * m->m[3][3];

    return out;
}

f32 FastAbs(f32 x)
{
    return x < 0.0f ? -x : x;
}

f32 FastLength2D(f32 x, f32 y)
{
    f32 ax;
    f32 ay;
    f32 maxv;
    f32 minv;

    ax = FastAbs(x);
    ay = FastAbs(y);

    if (ax > ay)
    {
        maxv = ax;
        minv = ay;
    }
    else
    {
        maxv = ay;
        minv = ax;
    }

    return maxv + (minv * 0.375f);
}

Matrix4x4 MatrixMultiply(Matrix4x4 a, Matrix4x4 b)
{
    Matrix4x4 out;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            out.m[i][j] = 0.0f;
            for (int k = 0; k < 4; k++)
            {
                out.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return out;
}