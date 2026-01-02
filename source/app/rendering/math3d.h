#ifndef MATH3D_H
#define MATH3D_H

#include <stdint.h>
#include <math.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PI              3.14159265358979323846f
#define TWO_PI          6.28318530717958647692f
#define HALF_PI         1.57079632679489661923f
#define DEG_TO_RAD      0.01745329251994329577f
#define RAD_TO_DEG      57.2957795130823208768f
#define EPSILON         0.0001f

    /* Vector Types */
    typedef struct { float x, y; } Vec2;
    typedef struct { float x, y, z; } Vec3;
    typedef struct { float x, y, z, w; } Vec4;
    typedef struct { float m[9]; } Mat3;
    typedef struct { float m[16]; } Mat4;
    typedef struct { float x, y, z, w; } Quaternion;

    /* Utility */
    static inline float Clampf(float v, float lo, float hi) {
        return (v < lo) ? lo : ((v > hi) ? hi : v);
    }

    static inline int Clampi(int v, int lo, int hi) {
        return (v < lo) ? lo : ((v > hi) ? hi : v);
    }

    static inline float Lerpf(float a, float b, float t) {
        return a + (b - a) * t;
    }

    static inline int Min3i(int a, int b, int c) {
        int m = a; if (b < m) m = b; if (c < m) m = c; return m;
    }

    static inline int Max3i(int a, int b, int c) {
        int m = a; if (b > m) m = b; if (c > m) m = c; return m;
    }

    /* Vec2 */
    static inline Vec2 Vec2_Create(float x, float y) { Vec2 r; r.x = x; r.y = y; return r; }
    static inline Vec2 Vec2_Add(Vec2 a, Vec2 b) { Vec2 r; r.x = a.x + b.x; r.y = a.y + b.y; return r; }
    static inline Vec2 Vec2_Sub(Vec2 a, Vec2 b) { Vec2 r; r.x = a.x - b.x; r.y = a.y - b.y; return r; }
    static inline Vec2 Vec2_Scale(Vec2 v, float s) { Vec2 r; r.x = v.x * s; r.y = v.y * s; return r; }
    static inline float Vec2_Dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
    static inline float Vec2_Length(Vec2 v) { return sqrtf(v.x * v.x + v.y * v.y); }
    static inline float Vec2_LengthSq(Vec2 v) { return v.x * v.x + v.y * v.y; }

    static inline Vec2 Vec2_Normalize(Vec2 v) {
        Vec2 r; float len = Vec2_Length(v);
        if (len > EPSILON) { float inv = 1.0f / len; r.x = v.x * inv; r.y = v.y * inv; return r; }
        r.x = 0; r.y = 0; return r;
    }

    /* Vec3 */
    static inline Vec3 Vec3_Create(float x, float y, float z) { Vec3 r; r.x = x; r.y = y; r.z = z; return r; }
    static inline Vec3 Vec3_Zero(void) { Vec3 r; r.x = 0; r.y = 0; r.z = 0; return r; }
    static inline Vec3 Vec3_One(void) { Vec3 r; r.x = 1; r.y = 1; r.z = 1; return r; }
    static inline Vec3 Vec3_Up(void) { Vec3 r; r.x = 0; r.y = 1; r.z = 0; return r; }
    static inline Vec3 Vec3_Forward(void) { Vec3 r; r.x = 0; r.y = 0; r.z = -1; return r; }
    static inline Vec3 Vec3_Right(void) { Vec3 r; r.x = 1; r.y = 0; r.z = 0; return r; }

    static inline Vec3 Vec3_Add(Vec3 a, Vec3 b) { Vec3 r; r.x = a.x + b.x; r.y = a.y + b.y; r.z = a.z + b.z; return r; }
    static inline Vec3 Vec3_Sub(Vec3 a, Vec3 b) { Vec3 r; r.x = a.x - b.x; r.y = a.y - b.y; r.z = a.z - b.z; return r; }
    static inline Vec3 Vec3_Mul(Vec3 a, Vec3 b) { Vec3 r; r.x = a.x * b.x; r.y = a.y * b.y; r.z = a.z * b.z; return r; }
    static inline Vec3 Vec3_Scale(Vec3 v, float s) { Vec3 r; r.x = v.x * s; r.y = v.y * s; r.z = v.z * s; return r; }
    static inline Vec3 Vec3_Negate(Vec3 v) { Vec3 r; r.x = -v.x; r.y = -v.y; r.z = -v.z; return r; }
    static inline float Vec3_Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    static inline float Vec3_Length(Vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
    static inline float Vec3_LengthSq(Vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

    static inline Vec3 Vec3_Cross(Vec3 a, Vec3 b) {
        Vec3 result;
        result.x = a.y * b.z - a.z * b.y;
        result.y = a.z * b.x - a.x * b.z;
        result.z = a.x * b.y - a.y * b.x;
        return result;
    }

    static inline Vec3 Vec3_Normalize(Vec3 v) {
        float len = Vec3_Length(v);
        if (len > EPSILON) {
            float inv = 1.0f / len;
            Vec3 result;
            result.x = v.x * inv;
            result.y = v.y * inv;
            result.z = v.z * inv;
            return result;
        }
        Vec3 zero;
        zero.x = 0.0f;
        zero.y = 0.0f;
        zero.z = 0.0f;
        return zero;
    }

    static inline Vec3 Vec3_Lerp(Vec3 a, Vec3 b, float t) {
        Vec3 result;
        result.x = a.x + (b.x - a.x) * t;
        result.y = a.y + (b.y - a.y) * t;
        result.z = a.z + (b.z - a.z) * t;
        return result;
    }

    static inline float Vec3_Distance(Vec3 a, Vec3 b) { return Vec3_Length(Vec3_Sub(a, b)); }

    static inline Vec3 Vec3_Reflect(Vec3 v, Vec3 n) {
        return Vec3_Sub(v, Vec3_Scale(n, 2.0f * Vec3_Dot(v, n)));
    }

    static inline Vec3 Vec3_Min(Vec3 a, Vec3 b) {
        Vec3 result;
        result.x = (a.x < b.x) ? a.x : b.x;
        result.y = (a.y < b.y) ? a.y : b.y;
        result.z = (a.z < b.z) ? a.z : b.z;
        return result;
    }

    static inline Vec3 Vec3_Max(Vec3 a, Vec3 b) {
        Vec3 result;
        result.x = (a.x > b.x) ? a.x : b.x;
        result.y = (a.y > b.y) ? a.y : b.y;
        result.z = (a.z > b.z) ? a.z : b.z;
        return result;
    }

    /* Vec4 */
    static inline Vec4 Vec4_Create(float x, float y, float z, float w) { Vec4 r; r.x = x; r.y = y; r.z = z; r.w = w; return r; }
    static inline Vec4 Vec4_FromVec3(Vec3 v, float w) { Vec4 result; result.x = v.x; result.y = v.y; result.z = v.z; result.w = w; return result; }
    static inline Vec3 Vec4_ToVec3(Vec4 v) { Vec3 r; r.x = v.x; r.y = v.y; r.z = v.z; return r; }
    static inline Vec4 Vec4_Add(Vec4 a, Vec4 b) { Vec4 r; r.x = a.x + b.x; r.y = a.y + b.y; r.z = a.z + b.z; r.w = a.w + b.w; return r; }
    static inline Vec4 Vec4_Scale(Vec4 v, float s) { Vec4 r; r.x = v.x * s; r.y = v.y * s; r.z = v.z * s; r.w = v.w * s; return r; }
    static inline float Vec4_Dot(Vec4 a, Vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

    /* Mat4 */
    static inline void Mat4_Identity(Mat4* m) {
        memset(m->m, 0, sizeof(m->m));
        m->m[0] = m->m[5] = m->m[10] = m->m[15] = 1.0f;
    }

    static inline void Mat4_Copy(Mat4* dst, const Mat4* src) {
        memcpy(dst->m, src->m, sizeof(dst->m));
    }

    static inline void Mat4_Multiply(Mat4* out, const Mat4* a, const Mat4* b) {
        Mat4 r;
        int row, col;
        for (row = 0; row < 4; row++) {
            for (col = 0; col < 4; col++) {
                r.m[col * 4 + row] =
                    a->m[0 * 4 + row] * b->m[col * 4 + 0] +
                    a->m[1 * 4 + row] * b->m[col * 4 + 1] +
                    a->m[2 * 4 + row] * b->m[col * 4 + 2] +
                    a->m[3 * 4 + row] * b->m[col * 4 + 3];
            }
        }
        *out = r;
    }

    static inline Vec4 Mat4_MultiplyVec4(const Mat4* m, Vec4 v) {
        Vec4 r;
        r.x = m->m[0] * v.x + m->m[4] * v.y + m->m[8] * v.z + m->m[12] * v.w;
        r.y = m->m[1] * v.x + m->m[5] * v.y + m->m[9] * v.z + m->m[13] * v.w;
        r.z = m->m[2] * v.x + m->m[6] * v.y + m->m[10] * v.z + m->m[14] * v.w;
        r.w = m->m[3] * v.x + m->m[7] * v.y + m->m[11] * v.z + m->m[15] * v.w;
        return r;
    }

    static inline Vec3 Mat4_TransformPoint(const Mat4* m, Vec3 p) {
        Vec3 r;
        r.x = m->m[0] * p.x + m->m[4] * p.y + m->m[8] * p.z + m->m[12];
        r.y = m->m[1] * p.x + m->m[5] * p.y + m->m[9] * p.z + m->m[13];
        r.z = m->m[2] * p.x + m->m[6] * p.y + m->m[10] * p.z + m->m[14];
        return r;
    }

    static inline Vec3 Mat4_TransformVector(const Mat4* m, Vec3 v) {
        Vec3 r;
        r.x = m->m[0] * v.x + m->m[4] * v.y + m->m[8] * v.z;
        r.y = m->m[1] * v.x + m->m[5] * v.y + m->m[9] * v.z;
        r.z = m->m[2] * v.x + m->m[6] * v.y + m->m[10] * v.z;
        return r;
    }

    static inline void Mat4_Translation(Mat4* m, float x, float y, float z) {
        Mat4_Identity(m);
        m->m[12] = x; m->m[13] = y; m->m[14] = z;
    }

    static inline void Mat4_Scale(Mat4* m, float x, float y, float z) {
        Mat4_Identity(m);
        m->m[0] = x; m->m[5] = y; m->m[10] = z;
    }

    static inline void Mat4_RotationX(Mat4* m, float rad) {
        float c = cosf(rad), s = sinf(rad);
        Mat4_Identity(m);
        m->m[5] = c; m->m[9] = -s; m->m[6] = s; m->m[10] = c;
    }

    static inline void Mat4_RotationY(Mat4* m, float rad) {
        float c = cosf(rad), s = sinf(rad);
        Mat4_Identity(m);
        m->m[0] = c; m->m[8] = s; m->m[2] = -s; m->m[10] = c;
    }

    static inline void Mat4_RotationZ(Mat4* m, float rad) {
        float c = cosf(rad), s = sinf(rad);
        Mat4_Identity(m);
        m->m[0] = c; m->m[4] = -s; m->m[1] = s; m->m[5] = c;
    }

    static inline void Mat4_Perspective(Mat4* m, float fov_rad, float aspect, float near, float far) {
        float tan_half = tanf(fov_rad * 0.5f);
        memset(m->m, 0, sizeof(m->m));
        m->m[0] = 1.0f / (aspect * tan_half);
        m->m[5] = 1.0f / tan_half;
        m->m[10] = far / (near - far);
        m->m[11] = -1.0f;
        m->m[14] = (far * near) / (near - far);
    }

    static inline void Mat4_Orthographic(Mat4* m, float l, float r, float b, float t, float n, float f) {
        memset(m->m, 0, sizeof(m->m));
        m->m[0] = 2.0f / (r - l);
        m->m[5] = 2.0f / (t - b);
        m->m[10] = -2.0f / (f - n);
        m->m[12] = -(r + l) / (r - l);
        m->m[13] = -(t + b) / (t - b);
        m->m[14] = -(f + n) / (f - n);
        m->m[15] = 1.0f;
    }

    static inline void Mat4_LookAt(Mat4* m, Vec3 eye, Vec3 target, Vec3 up) {
        Vec3 f = Vec3_Normalize(Vec3_Sub(target, eye));
        Vec3 r = Vec3_Normalize(Vec3_Cross(f, up));
        Vec3 u = Vec3_Cross(r, f);

        m->m[0] = r.x;  m->m[4] = r.y;  m->m[8] = r.z;   m->m[12] = -Vec3_Dot(r, eye);
        m->m[1] = u.x;  m->m[5] = u.y;  m->m[9] = u.z;   m->m[13] = -Vec3_Dot(u, eye);
        m->m[2] = -f.x; m->m[6] = -f.y; m->m[10] = -f.z; m->m[14] = Vec3_Dot(f, eye);
        m->m[3] = 0;    m->m[7] = 0;    m->m[11] = 0;    m->m[15] = 1;
    }

    static inline void Mat4_InverseRigid(Mat4* out, const Mat4* m) {
        Vec3 t;
        out->m[0] = m->m[0]; out->m[1] = m->m[4]; out->m[2] = m->m[8]; out->m[3] = 0;
        out->m[4] = m->m[1]; out->m[5] = m->m[5]; out->m[6] = m->m[9]; out->m[7] = 0;
        out->m[8] = m->m[2]; out->m[9] = m->m[6]; out->m[10] = m->m[10]; out->m[11] = 0;
        t.x = -m->m[12]; t.y = -m->m[13]; t.z = -m->m[14];
        out->m[12] = out->m[0] * t.x + out->m[4] * t.y + out->m[8] * t.z;
        out->m[13] = out->m[1] * t.x + out->m[5] * t.y + out->m[9] * t.z;
        out->m[14] = out->m[2] * t.x + out->m[6] * t.y + out->m[10] * t.z;
        out->m[15] = 1;
    }

    /* Quaternion */
    static inline Quaternion Quat_Identity(void) { Quaternion q; q.x = 0; q.y = 0; q.z = 0; q.w = 1; return q; }

    static inline Quaternion Quat_FromAxisAngle(Vec3 axis, float rad) {
        float half = rad * 0.5f, s = sinf(half);
        Vec3 n = Vec3_Normalize(axis);
        Quaternion q;
        q.x = n.x * s;
        q.y = n.y * s;
        q.z = n.z * s;
        q.w = cosf(half);
        return q;
    }

    static inline float Quat_Length(Quaternion q) {
        return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    }

    static inline Quaternion Quat_Normalize(Quaternion q) {
        float len = Quat_Length(q);
        if (len > EPSILON) {
            float inv = 1.0f / len;
            Quaternion result;
            result.x = q.x * inv;
            result.y = q.y * inv;
            result.z = q.z * inv;
            result.w = q.w * inv;
            return result;
        }
        return Quat_Identity();
    }

    static inline Quaternion Quat_Multiply(Quaternion a, Quaternion b) {
        Quaternion result;
        result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
        result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
        result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
        result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
        return result;
    }

    static inline Vec3 Quat_RotateVec3(Quaternion q, Vec3 v) {
        Vec3 qv;
        Vec3 uv, uuv;
        qv.x = q.x; qv.y = q.y; qv.z = q.z;
        uv = Vec3_Cross(qv, v);
        uuv = Vec3_Cross(qv, uv);
        return Vec3_Add(v, Vec3_Scale(Vec3_Add(Vec3_Scale(uv, q.w), uuv), 2.0f));
    }

    static inline void Quat_ToMat4(Mat4* m, Quaternion q) {
        float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
        float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
        float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

        m->m[0] = 1 - 2 * (yy + zz); m->m[1] = 2 * (xy + wz);   m->m[2] = 2 * (xz - wy);   m->m[3] = 0;
        m->m[4] = 2 * (xy - wz);   m->m[5] = 1 - 2 * (xx + zz); m->m[6] = 2 * (yz + wx);   m->m[7] = 0;
        m->m[8] = 2 * (xz + wy);   m->m[9] = 2 * (yz - wx);   m->m[10] = 1 - 2 * (xx + yy); m->m[11] = 0;
        m->m[12] = 0;          m->m[13] = 0;          m->m[14] = 0;           m->m[15] = 1;
    }

    static inline Quaternion Quat_Slerp(Quaternion a, Quaternion b, float t) {
        float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        float theta0, theta, sin_theta, sin_theta0, s0, s1;
        Quaternion result;

        if (dot < 0) { b.x = -b.x; b.y = -b.y; b.z = -b.z; b.w = -b.w; dot = -dot; }
        if (dot > 0.9995f) {
            result.x = a.x + (b.x - a.x) * t;
            result.y = a.y + (b.y - a.y) * t;
            result.z = a.z + (b.z - a.z) * t;
            result.w = a.w + (b.w - a.w) * t;
            return Quat_Normalize(result);
        }
        theta0 = acosf(dot);
        theta = theta0 * t;
        sin_theta = sinf(theta);
        sin_theta0 = sinf(theta0);
        s0 = cosf(theta) - dot * sin_theta / sin_theta0;
        s1 = sin_theta / sin_theta0;
        result.x = a.x * s0 + b.x * s1;
        result.y = a.y * s0 + b.y * s1;
        result.z = a.z * s0 + b.z * s1;
        result.w = a.w * s0 + b.w * s1;
        return result;
    }

#ifdef __cplusplus
}
#endif

#endif /* MATH3D_H */