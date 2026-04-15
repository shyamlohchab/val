#pragma once
#include <cmath>
#include "sdk.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Math
{
    inline float Dot(const FVector& a, const FVector& b)
    {
        return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
    }

    inline float Length(const FVector& v)
    {
        return sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
    }

    inline FVector Normalize(const FVector& v)
    {
        float len = Length(v);
        if (len < 1e-6f) return {};
        return { v.X / len, v.Y / len, v.Z / len };
    }

    inline float Distance(const FVector& a, const FVector& b)
    {
        return Length(a - b);
    }

    // Converts a world-space delta vector to Pitch/Yaw angles
    inline FRotator VectorToRotator(const FVector& dir)
    {
        FRotator rot{};
        rot.Yaw   = atan2f(dir.Y, dir.X) * (180.f / static_cast<float>(M_PI));
        rot.Pitch = atan2f(-dir.Z, sqrtf(dir.X * dir.X + dir.Y * dir.Y))
                    * (180.f / static_cast<float>(M_PI));
        rot.Roll  = 0.f;
        return rot;
    }

    // Projects a world-space point onto the screen using the view-projection matrix.
    // Returns false if the point is behind the camera.
    inline bool WorldToScreen(const FVector& world, const FMatrix& vp,
                              float screenW, float screenH,
                              FVector2D& out)
    {
        float w = vp.M[0][3] * world.X + vp.M[1][3] * world.Y
                + vp.M[2][3] * world.Z + vp.M[3][3];
        if (w < 0.01f) return false;

        float x = vp.M[0][0] * world.X + vp.M[1][0] * world.Y
                + vp.M[2][0] * world.Z + vp.M[3][0];
        float y = vp.M[0][1] * world.X + vp.M[1][1] * world.Y
                + vp.M[2][1] * world.Z + vp.M[3][1];

        out.X = (screenW / 2.f) + (screenW / 2.f) * x / w;
        out.Y = (screenH / 2.f) - (screenH / 2.f) * y / w;
        return true;
    }

    // Clamps angle to [-180, 180]
    inline float ClampAngle(float angle)
    {
        while (angle > 180.f)  angle -= 360.f;
        while (angle < -180.f) angle += 360.f;
        return angle;
    }

    // Smooths a rotation delta by a factor (0 = instant, higher = smoother)
    inline FRotator SmoothAim(const FRotator& current, const FRotator& target, float smooth)
    {
        FRotator result;
        result.Pitch = current.Pitch + ClampAngle(target.Pitch - current.Pitch) / smooth;
        result.Yaw   = current.Yaw   + ClampAngle(target.Yaw   - current.Yaw)   / smooth;
        result.Roll  = 0.f;
        return result;
    }
}
