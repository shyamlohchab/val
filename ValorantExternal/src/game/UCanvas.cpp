#include "UCanvas.h"
#include "../math.h"

// ============================================================
//  UCanvas / Draw Helpers
//  Thin wrappers around ImGui draw list for common ESP shapes.
// ============================================================

namespace Draw
{
    static ImDrawList* GetDL() { return ImGui::GetBackgroundDrawList(); }

    void Line(FVector2D from, FVector2D to, ImU32 color, float thickness)
    {
        GetDL()->AddLine({ from.X, from.Y }, { to.X, to.Y }, color, thickness);
    }

    void Rect(FVector2D topLeft, FVector2D bottomRight, ImU32 color, float thickness)
    {
        GetDL()->AddRect({ topLeft.X, topLeft.Y },
                         { bottomRight.X, bottomRight.Y }, color, 0.f, 0, thickness);
    }

    void FilledRect(FVector2D topLeft, FVector2D bottomRight, ImU32 color)
    {
        GetDL()->AddRectFilled({ topLeft.X, topLeft.Y },
                               { bottomRight.X, bottomRight.Y }, color);
    }

    void Circle(FVector2D center, float radius, ImU32 color, int segments)
    {
        GetDL()->AddCircle({ center.X, center.Y }, radius, color, segments);
    }

    void Text(FVector2D pos, ImU32 color, const char* text)
    {
        GetDL()->AddText({ pos.X, pos.Y }, color, text);
    }

    void Box3D(const FVector2D corners[8], ImU32 color, float thickness)
    {
        // Bottom face
        for (int i = 0; i < 4; ++i)
            Line(corners[i], corners[(i + 1) % 4], color, thickness);
        // Top face
        for (int i = 4; i < 8; ++i)
            Line(corners[i], corners[4 + (i + 1) % 4], color, thickness);
        // Verticals
        for (int i = 0; i < 4; ++i)
            Line(corners[i], corners[i + 4], color, thickness);
    }

    void Bone(FVector2D a, FVector2D b, ImU32 color)
    {
        Line(a, b, color, 1.5f);
    }

    void HealthBar(FVector2D topLeft, float boxHeight, float healthPct,
                   ImU32 bgColor, ImU32 fgColor)
    {
        float filled = boxHeight * healthPct;
        FilledRect({ topLeft.X - 6.f, topLeft.Y },
                   { topLeft.X - 3.f, topLeft.Y + boxHeight }, bgColor);
        FilledRect({ topLeft.X - 6.f, topLeft.Y + boxHeight - filled },
                   { topLeft.X - 3.f, topLeft.Y + boxHeight }, fgColor);
    }
}
