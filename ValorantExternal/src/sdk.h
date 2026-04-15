#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include "offsets.h"
#include "decryption.h"

// ============================================================
//  SDK — Unreal Engine 4 / Valorant type wrappers
// ============================================================

struct FVector
{
    float X, Y, Z;
    FVector() : X(0), Y(0), Z(0) {}
    FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}
    FVector operator-(const FVector& o) const { return { X - o.X, Y - o.Y, Z - o.Z }; }
    FVector operator+(const FVector& o) const { return { X + o.X, Y + o.Y, Z + o.Z }; }
    FVector operator*(float s)          const { return { X * s, Y * s, Z * s }; }
};

struct FVector2D
{
    float X, Y;
    FVector2D() : X(0), Y(0) {}
    FVector2D(float x, float y) : X(x), Y(y) {}
};

struct FRotator
{
    float Pitch, Yaw, Roll;
};

struct FQuat
{
    float X, Y, Z, W;
};

struct FTransform
{
    FQuat  Rotation;
    FVector Translation;
    float  pad;
    FVector Scale3D;
    float  pad2;

    // Returns the world-space position of a bone
    FVector GetBoneLocation() const { return Translation; }
};

struct FMatrix
{
    float M[4][4];
};

// Minimal UObject wrapper
struct UObject
{
    uintptr_t vtable;   // 0x00
    uint8_t   pad[0x08]; // 0x08
    int32_t   ObjectFlags; // 0x10 (approx)
    int32_t   InternalIndex;
    uintptr_t ClassPrivate;
    uintptr_t NamePrivate;
    uintptr_t OuterPrivate;
};

// TArray wrapper
template<typename T>
struct TArray
{
    uintptr_t Data;
    int32_t   Count;
    int32_t   Max;

    T Read(int index) const
    {
        return *reinterpret_cast<T*>(Data + index * sizeof(T));
    }
};

// FString
struct FString
{
    uintptr_t Data;
    int32_t   Count;
    int32_t   Max;

    std::wstring ToWString() const
    {
        if (!Data || Count <= 0) return L"";
        return std::wstring(reinterpret_cast<wchar_t*>(Data), Count - 1);
    }

    std::string ToString() const
    {
        auto ws = ToWString();
        return std::string(ws.begin(), ws.end());
    }
};
