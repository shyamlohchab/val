#include "esp.h"
#include "../sdk.h"
#include "../offsets.h"
#include "../decryption.h"
#include "../math.h"
#include <imgui/imgui.h>
#include <vector>
#include <string>

namespace ESP
{
    Config g_cfg;

    struct PlayerEntry
    {
        FVector2D headScreen;
        FVector2D feetScreen;
        float     health;
        float     distance;
        std::string name;
        bool      isEnemy;
        bool      isVisible;
        bool      onScreen;
    };

    static std::vector<PlayerEntry> g_players;

    // Placeholder view-projection matrix — populate from UGameViewportClient
    static FMatrix g_vpMatrix{};
    static float   g_screenW = 1920.f;
    static float   g_screenH = 1080.f;

    void Tick()
    {
        g_players.clear();
        if (!g_cfg.enabled) return;

        uintptr_t gworld = Decryption::GetGWorld();
        if (!gworld) return;

        // Use offset constant instead of hardcoded magic
        uintptr_t gameState = *reinterpret_cast<uintptr_t*>(gworld + Offsets::UWORLD_GAME_STATE);
        auto& playerArray   = *reinterpret_cast<TArray<uintptr_t>*>(
            gameState + Offsets::GAMESTATE_PLAYER_ARRAY);

        for (int i = 0; i < playerArray.Count; ++i)
        {
            uintptr_t ps = playerArray.Read(i);
            if (!ps) continue;

            uintptr_t pawn = *reinterpret_cast<uintptr_t*>(ps + 0x308);
            if (!pawn) continue;

            uintptr_t mesh = *reinterpret_cast<uintptr_t*>(pawn + Offsets::PAWN_MESH);
            if (!mesh) continue;

            // Read bone positions
            auto readBone = [&](int idx) -> FVector {
                uintptr_t arr = *reinterpret_cast<uintptr_t*>(
                    mesh + Offsets::MESH_COMPONENT_SPACE_BASES);
                if (!arr) return {};
                return reinterpret_cast<FTransform*>(arr + idx * sizeof(FTransform))
                           ->GetBoneLocation();
            };

            FVector head = readBone(Offsets::BONE_HEAD);
            FVector feet = readBone(Offsets::BONE_PELVIS);

            PlayerEntry entry{};
            entry.isEnemy  = true; // TODO: team comparison
            entry.health   = *reinterpret_cast<float*>(pawn + Offsets::PAWN_HEALTH);
            entry.distance = Math::Distance(head, {}) / 100.f; // cm -> m

            if (entry.distance > g_cfg.maxDistance) continue;

            entry.onScreen = Math::WorldToScreen(head, g_vpMatrix,
                                 g_screenW, g_screenH, entry.headScreen)
                          && Math::WorldToScreen(feet, g_vpMatrix,
                                 g_screenW, g_screenH, entry.feetScreen);

            // Name from PlayerState
            auto nameStr = reinterpret_cast<FString*>(ps + Offsets::PLAYER_STATE_NAME);
            entry.name = nameStr->ToString();

            g_players.push_back(entry);
        }
    }

    void Render()
    {
        if (!g_cfg.enabled || g_players.empty()) return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();

        for (auto& p : g_players)
        {
            if (!p.onScreen) continue;

            ImU32 col = p.isEnemy
                ? IM_COL32(255, 0, 0, 255)
                : IM_COL32(0, 255, 0, 255);

            // Bounding box
            if (g_cfg.boxes)
            {
                float h = p.feetScreen.Y - p.headScreen.Y;
                float w = h * 0.4f;
                dl->AddRect(
                    { p.headScreen.X - w, p.headScreen.Y },
                    { p.headScreen.X + w, p.feetScreen.Y },
                    col, 0.f, 0, 1.5f);
            }

            // Name
            if (g_cfg.names && !p.name.empty())
                dl->AddText({ p.headScreen.X, p.headScreen.Y - 14.f },
                    col, p.name.c_str());

            // Distance
            if (g_cfg.distance)
            {
                char distBuf[32];
                snprintf(distBuf, sizeof(distBuf), "%.0fm", p.distance);
                dl->AddText({ p.headScreen.X, p.feetScreen.Y + 2.f },
                    IM_COL32(255, 255, 255, 200), distBuf);
            }

            // Health bar
            if (g_cfg.health)
            {
                float barH   = p.feetScreen.Y - p.headScreen.Y;
                float filled = (p.health / 100.f) * barH;
                dl->AddRectFilled(
                    { p.headScreen.X - 8.f, p.feetScreen.Y - filled },
                    { p.headScreen.X - 5.f, p.feetScreen.Y },
                    IM_COL32(0, 255, 0, 200));
            }
        }
    }
}
