#pragma once
#include "../sdk.h"
#include <imgui/imgui.h>

// ============================================================
//  Draw helpers — thin wrappers around ImGui draw list
// ============================================================
namespace Draw
{
    void Line(FVector2D from, FVector2D to, ImU32 color, float thickness = 1.f);
    void Rect(FVector2D topLeft, FVector2D bottomRight, ImU32 color, float thickness = 1.f);
    void FilledRect(FVector2D topLeft, FVector2D bottomRight, ImU32 color);
    void Circle(FVector2D center, float radius, ImU32 color, int segments = 32);
    void Text(FVector2D pos, ImU32 color, const char* text);
    void Box3D(const FVector2D corners[8], ImU32 color, float thickness = 1.f);
    void Bone(FVector2D a, FVector2D b, ImU32 color);
    void HealthBar(FVector2D topLeft, float boxHeight, float healthPct,
                   ImU32 bgColor, ImU32 fgColor);
}
