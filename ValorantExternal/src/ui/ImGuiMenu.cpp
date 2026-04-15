#include "../menu.h"
#include "../features/aimbot.h"
#include "../features/esp.h"
#include "../chams.h"
#include "../features/skinchanger.h"
#include <imgui/imgui.h>

namespace Menu
{
    bool g_open = false;

    void Init()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding    = 6.f;
        style.FrameRounding     = 4.f;
        style.GrabRounding      = 4.f;
        style.WindowBorderSize  = 1.f;
        style.FrameBorderSize   = 0.f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg]        = ImVec4(0.08f, 0.08f, 0.10f, 0.95f);
        colors[ImGuiCol_Header]          = ImVec4(0.20f, 0.20f, 0.25f, 1.f);
        colors[ImGuiCol_HeaderHovered]   = ImVec4(0.30f, 0.30f, 0.40f, 1.f);
        colors[ImGuiCol_Button]          = ImVec4(0.20f, 0.20f, 0.28f, 1.f);
        colors[ImGuiCol_ButtonHovered]   = ImVec4(0.30f, 0.30f, 0.42f, 1.f);
        colors[ImGuiCol_FrameBg]         = ImVec4(0.15f, 0.15f, 0.20f, 1.f);
        colors[ImGuiCol_CheckMark]       = ImVec4(0.40f, 0.70f, 1.00f, 1.f);
        colors[ImGuiCol_SliderGrab]      = ImVec4(0.40f, 0.70f, 1.00f, 1.f);
        colors[ImGuiCol_TitleBg]         = ImVec4(0.06f, 0.06f, 0.08f, 1.f);
        colors[ImGuiCol_TitleBgActive]   = ImVec4(0.10f, 0.10f, 0.14f, 1.f);
    }

    void Render()
    {
        if (!g_open) return;

        ImGui::SetNextWindowSize({ 480.f, 520.f }, ImGuiCond_Once);
        ImGui::Begin("Valorant External", &g_open,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        if (ImGui::BeginTabBar("MainTabs"))
        {
            // ---- Aimbot tab ----
            if (ImGui::BeginTabItem("Aimbot"))
            {
                ImGui::Checkbox("Enable Aimbot",    &Aimbot::g_cfg.enabled);
                ImGui::Checkbox("Visible Only",     &Aimbot::g_cfg.visibleOnly);
                ImGui::Checkbox("Triggerbot",       &Aimbot::g_cfg.triggerbot);
                ImGui::SliderFloat("FOV",           &Aimbot::g_cfg.fov,   1.f, 45.f);
                ImGui::SliderFloat("Smooth",        &Aimbot::g_cfg.smooth, 1.f, 20.f);
                ImGui::SliderFloat("Trigger Delay", &Aimbot::g_cfg.triggerDelay, 0.f, 0.5f);
                const char* bones[] = { "Head", "Neck", "Chest", "Pelvis" };
                int boneIdx = (Aimbot::g_cfg.targetBone == 8) ? 0
                            : (Aimbot::g_cfg.targetBone == 7) ? 1
                            : (Aimbot::g_cfg.targetBone == 5) ? 2 : 3;
                if (ImGui::Combo("Target Bone", &boneIdx, bones, 4))
                {
                    int map[] = { 8, 7, 5, 0 };
                    Aimbot::g_cfg.targetBone = map[boneIdx];
                }
                ImGui::EndTabItem();
            }

            // ---- ESP tab ----
            if (ImGui::BeginTabItem("ESP"))
            {
                ImGui::Checkbox("Enable ESP",     &ESP::g_cfg.enabled);
                ImGui::Checkbox("Boxes",           &ESP::g_cfg.boxes);
                ImGui::Checkbox("Skeleton",        &ESP::g_cfg.skeleton);
                ImGui::Checkbox("Names",           &ESP::g_cfg.names);
                ImGui::Checkbox("Health Bar",      &ESP::g_cfg.health);
                ImGui::Checkbox("Distance",        &ESP::g_cfg.distance);
                ImGui::Checkbox("Snaplines",       &ESP::g_cfg.snaplines);
                ImGui::Checkbox("Visible Check",   &ESP::g_cfg.visibleCheck);
                ImGui::SliderFloat("Max Distance", &ESP::g_cfg.maxDistance, 50.f, 1000.f);
                ImGui::ColorEdit4("Enemy Color",   ESP::g_cfg.enemyColor);
                ImGui::ColorEdit4("Team Color",    ESP::g_cfg.teamColor);
                ImGui::EndTabItem();
            }

            // ---- Chams tab ----
            if (ImGui::BeginTabItem("Chams"))
            {
                ImGui::Checkbox("Enable Chams",    &Chams::g_cfg.enabled);
                ImGui::Checkbox("Visible Only",    &Chams::g_cfg.visibleOnly);
                ImGui::Checkbox("Wireframe",       &Chams::g_cfg.wireframe);
                ImGui::ColorEdit4("Enemy Color",   Chams::g_cfg.enemyColor);
                ImGui::ColorEdit4("Team Color",    Chams::g_cfg.teamColor);
                ImGui::ColorEdit4("Hidden Color",  Chams::g_cfg.hiddenColor);
                ImGui::EndTabItem();
            }

            // ---- Skin Changer tab ----
            if (ImGui::BeginTabItem("Skins"))
            {
                ImGui::Checkbox("Enable Skin Changer", &SkinChanger::g_cfg.enabled);
                if (ImGui::Button("Apply Skins"))
                    SkinChanger::Apply();
                ImGui::SameLine();
                if (ImGui::Button("Revert Skins"))
                    SkinChanger::Revert();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void Shutdown()
    {
        // ImGui shutdown is handled by DX12Hook::Shutdown
    }
}
