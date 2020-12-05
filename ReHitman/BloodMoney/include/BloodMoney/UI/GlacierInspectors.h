#pragma once

#include <BloodMoney/UI/ImGuiInspector.h>
#include <Glacier/ZHumanBoid.h>
#include <Glacier/ZGROUP.h>
#include <Glacier/ZGEOM.h>
#include <Glacier/ZMath.h>

#include <spdlog/spdlog.h> //only for FMT
#include <imgui.h>

namespace ImGui
{
    // Custom impls
    template <> struct Inspector<Glacier::ZVector2> {
        static void Draw(const char* id, Glacier::ZVector2* data)
        {
            ImGui::InputFloat2(id, reinterpret_cast<float*>(data));
        }
    };

    template <> struct Inspector<Glacier::ZVector3> {
        static void Draw(const char* id, Glacier::ZVector3* data)
        {
            ImGui::InputFloat3(id, reinterpret_cast<float*>(data));
        }
    };

    template <> struct Inspector<Glacier::ZVector4> {
        static void Draw(const char* id, Glacier::ZVector4* data)
        {
            ImGui::InputFloat4(id, reinterpret_cast<float*>(data));
        }
    };

    template <> struct Inspector<Glacier::ZMat3x3> {
        static void Draw(const char* id, Glacier::ZMat3x3* data)
        {
            std::string labels[3] = {
                    fmt::format("M11 M21 M31 | {}", id),
                    fmt::format("M12 M22 M32 | {}", id),
                    fmt::format("M13 M23 M33 | {}", id)
            };

            Inspector<Glacier::ZVector3>::Draw(labels[0].data(), reinterpret_cast<Glacier::ZVector3*>(&data->data[0]));
            Inspector<Glacier::ZVector3>::Draw(labels[1].data(), reinterpret_cast<Glacier::ZVector3*>(&data->data[3]));
            Inspector<Glacier::ZVector3>::Draw(labels[2].data(), reinterpret_cast<Glacier::ZVector3*>(&data->data[6]));
        }
    };

    template <> struct Inspector<Glacier::ZMat4x4> {
        static void Draw(const char* id, Glacier::ZMat4x4* data)
        {
            std::string labels[4] = {
                    fmt::format("M11 M21 M31 M41 | {}", id),
                    fmt::format("M12 M22 M32 M42 | {}", id),
                    fmt::format("M13 M23 M33 M43 | {}", id),
                    fmt::format("M14 M24 M34 M44 | {}", id)
            };

            Inspector<Glacier::ZVector4>::Draw(labels[0].data(), reinterpret_cast<Glacier::ZVector4*>(&data->data[0]));
            Inspector<Glacier::ZVector4>::Draw(labels[1].data(), reinterpret_cast<Glacier::ZVector4*>(&data->data[4]));
            Inspector<Glacier::ZVector4>::Draw(labels[2].data(), reinterpret_cast<Glacier::ZVector4*>(&data->data[8]));
            Inspector<Glacier::ZVector4>::Draw(labels[3].data(), reinterpret_cast<Glacier::ZVector4*>(&data->data[12]));
        }
    };

    template <> struct Inspector<Glacier::EBoidState>
    {
        static void Draw(const char*, Glacier::EBoidState* state)
        {
            if (!state)
            {
                ImGui::TextColored(ImVec4 { 1.f, 0.f, 0.f, 1.f }, "INVALID EBoidState INSTANCE");
                return;
            }

            auto st = (Glacier::EBoidState)*state;
            switch (st)
            {
                case Glacier::EBoidState::Moving:
                    ImGui::Text("EBoidState::Moving");
                    break;
                case Glacier::EBoidState::Staying:
                    ImGui::Text("EBoidState::Staying");
                    break;
                case Glacier::EBoidState::ParentActorDestroyed:
                    ImGui::Text("EBoidState::ParentActorDestroyed (Dead)");
                    break;
                case Glacier::EBoidState::Sitting:
                    ImGui::Text("EBoidState::Sitting");
                    break;
                case Glacier::EBoidState::Unk6:
                    ImGui::Text("EBoidState::Unk6");
                    break;
                default:
                    ImGui::Text("Unknown EBoidState (%.8X)", st);
                    break;
            }
        }
    };

    template <> struct Inspector<Glacier::ZHumanBoid>
    {
        static void Draw(const char* id, Glacier::ZHumanBoid* boid)
        {
            {
                if (!boid)
                {
                    ImGui::TextColored(ImVec4 { 1.f, 0.f, 0.f, 1.f }, "INVALID ZHumanBoid INSTANCE");
                    return;
                }

                ImGui::Inspector<Glacier::EBoidState>::Draw("Boid state", &boid->m_boidState);
                ImGui::Inspector<Glacier::ZVector3>::Draw("Boid Position", &boid->m_Position);
                ImGui::Text("Actual speed: %f / Speed: %f / Speed2: %f", boid->m_ActualSpeed, boid->m_speed, boid->m_speed2);
            }
        }
    };

    template <> struct Inspector<Glacier::ZGROUP>
    {
        static void Draw(const char* id, Glacier::ZGROUP* group)
        {
            if (!group)
            {
                ImGui::TextColored(ImVec4 { 1.f, 0.f, 0.f, 1.f }, "INVALID ZGROUP INSTANCE");
                return;
            }

            ImGui::Text(" GROUP INFO ");
            ImGui::Separator();

            ImGui::Text("Name   : %s", group->m_pEntityLocator->entityName);
            ImGui::Text("Depth  : %d", group->GroupDepth());
            ImGui::Text("IsRoot : %s", (group->IsRoot() ? "YES" : "NO"));

            if (!group->IsRoot())
            {
                auto parentGroup = group->m_pEntityLocator->ParentGroup();
                // Show parent node
                if (parentGroup && ImGui::TreeNode("Parent"))
                {
                    ImGui::Inspector<Glacier::ZGROUP>::Draw(parentGroup->m_pEntityLocator->entityName, parentGroup);
                    ImGui::TreePop();
                }
            }

            // Try to locate children nodes
            // For test: try to take a next geom

        }
    };
}