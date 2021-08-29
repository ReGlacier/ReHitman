#include <spdlog/spdlog.h>
#include <imgui.h>

#include <BloodMoney/UI/Widgets/SandboxWidget.h>
#include <BloodMoney/UI/GlacierInspectors.h>

#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3DialogControl.h>

#include <BloodMoney/Game/UI/ZHM3MenuElements.h>
#include <BloodMoney/Game/UI/ZHM3MenuFactory.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>
#include <BloodMoney/Game/UI/ZWINDOWS.h>
#include <BloodMoney/Game/UI/ZGUIBase.h>

#include <BloodMoney/Game/Globals.h>

#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Glacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/CInventory.h>
#include <Glacier/CConfiguration.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/ZTypeTraits.h>
#include <Glacier/ZRenderWintelD3D.h>

#include <BloodMoney/Game/ZHM3Actor.h>

#include <BloodMoney/Game/CIngameMap.h>
#include <BloodMoney/Game/ZHM3HmAs.h>
#include <BloodMoney/Game/ZHM3ClothBundle.h>
#include <BloodMoney/Game/UI/ZCHAROBJ.h>
#include <BloodMoney/Game/UI/ZWINOBJ.h>
#include <BloodMoney/Game/UI/ZXMLGUISystem.h>

#include <BloodMoney/Game/ZCloth.h>
#include <BloodMoney/Game/ZTie.h>
#include <BloodMoney/Game/UI/ZLINEOBJ.h>

#include <HF/HackingFramework.hpp>
#include <Glacier/ZRenderWintelD3DDll.h>
#include <Glacier/ZPrimControlWintel.h>
#include <Glacier/ZEngineGeomControl.h>
#include <Glacier/CCom.h>

#include <BloodMoney/Game/OnLevel/ZVCR.h>


namespace Hitman::BloodMoney
{

    void SandboxWidget::draw()
    {
        DebugWidget::draw();

        auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Globals::kGameDataAddr);
        if (!gameData) { return; }

        auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
        if (!sysInterface) { return; }

        auto engineDb = sysInterface->m_engineDataBase;
        if (!engineDb) { return; }

        auto sceneCom = engineDb->GetSceneCom();
        if (!sceneCom) { return; }

        auto renderDll = Glacier::getInterface<Glacier::ZRenderWintelD3DDll>(Globals::kD3DDllAddr);
        if (!renderDll) { return; }

        auto geomBuffer = Glacier::getInterface<Glacier::ZGeomBuffer>(Globals::kGeomBufferAddr);
        if (!geomBuffer) { return; }

        auto hitman = reinterpret_cast<Glacier::ZLNKWHANDS*>(gameData->m_Hitman3);
        if (!hitman) return;

        ImGui::Begin("Test script");
        if (ImGui::Button("Try to dump few prims")) {
            spdlog::info("Hitman   prim {:08X}", (int)renderDll->m_primControlWintel->GetPrimData(reinterpret_cast<Glacier::ZGEOM*>(gameData->m_Hitman3)->m_baseGeom->m_primitive));
            spdlog::info("Actor[0] prim {:08X}", (int)renderDll->m_primControlWintel->GetPrimData(gameData->m_ActorsPool[0]->m_baseGeom->m_primitive));
            spdlog::info("Actor[1] prim {:08X}", (int)renderDll->m_primControlWintel->GetPrimData(gameData->m_ActorsPool[1]->m_baseGeom->m_primitive));

            char buffer[512] { 0 };

            using GetPrimStats_t = int(__thiscall*)(int, char*, int, int);
            auto GetPrimStats = (GetPrimStats_t)0x00487740;

            auto r = GetPrimStats((int)renderDll->m_primControlWintel, &buffer[0], 511, reinterpret_cast<Glacier::ZGEOM*>(gameData->m_Hitman3)->m_baseGeom->m_primitive);
            spdlog::info("R: {:08X}| {}", r, buffer);
            std::memset(&buffer[0], 0x0, 512);

            for (int i = 0; i < gameData->m_ActorsInPoolCount; i++) {
                GetPrimStats((int)renderDll->m_primControlWintel, &buffer[0], 511, gameData->m_ActorsPool[i]->m_baseGeom->m_primitive);
                spdlog::info("Actor[{}]: {}", i, buffer);
                std::memset(&buffer[0], 0x0, 512);
            }
        }

        ImGui::End();

        //-----------------------------------------------------------------------------------------------
        ImGui::Begin("TEST");
        if (ImGui::Button("Test extract vars")) {
            auto& ccom = *engineDb->GetSceneCom();
            Glacier::REFTAB* ccomVars = &engineDb->GetSceneCom()->m_variables;

            for (int i = 0; i < ccomVars->Count(); i++) {
                auto pEnt = reinterpret_cast<Glacier::CCom::Ent*>(ccomVars->operator[](i));
                const char* sEntName = pEnt->GetKey();
                std::intptr_t iValue = pEnt->GetValue();

                switch (pEnt->m_pTypeInfo->m_eKind) {
                    case Glacier::CCom::Ent::TypeInfo::EKind::Bool:
                        spdlog::info("[{}] '{}' = {}", i, sEntName, static_cast<bool>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Char:
                        spdlog::info("[{}] '{}' = {}", i, sEntName, static_cast<char>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Long:
                        spdlog::info("[{}] '{}' = {}", i, sEntName, static_cast<int>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Float32:
                        spdlog::info("[{}] '{}' = {}", i, sEntName, static_cast<float>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Real:
                        spdlog::info("[{}] '{}' = {:X} (REAL)", i, sEntName, iValue);
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::P:
                        spdlog::info("[{}] '{}' = {:X}", i, sEntName, static_cast<std::intptr_t>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Vec3:
                        spdlog::info("[{}] '{}' = VEC3? {:X}", i, sEntName, static_cast<std::intptr_t>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Vec3F:
                        spdlog::info("[{}] '{}' = VEC3F? {:X}", i, sEntName, static_cast<std::intptr_t>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Mat33:
                        spdlog::info("[{}] '{}' = MAT3x3? {:X}", i, sEntName, static_cast<std::intptr_t>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Mat33F:
                        spdlog::info("[{}] '{}' = MAT3x3F? {:X}", i, sEntName, static_cast<std::intptr_t>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Str:
                        spdlog::info("[{}] '{}' = {}", i, sEntName, reinterpret_cast<const char*>(&iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Blk:
                        spdlog::info("[{}] '{}' = BLK? {:X}", i, sEntName, static_cast<std::intptr_t>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::File:
                        spdlog::info("[{}] '{}' = FILE? {:X}", i, sEntName, static_cast<std::intptr_t>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::Data:
                        spdlog::info("[{}] '{}' = DATA? {:X}", i, sEntName, static_cast<std::intptr_t>(iValue));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::GREF:
                        spdlog::info("[{}] '{}' = {:8X} (GEOM REF)", i, sEntName, reinterpret_cast<std::intptr_t>(Glacier::ZGEOM::RefToPtr(iValue)));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::GRFT:
                        spdlog::info("[{}] '{}' = {:8X} (REF TAB)", i, sEntName, reinterpret_cast<std::intptr_t>(Glacier::ZGEOM::RefToPtr(iValue)));
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::ZMSG:
                        spdlog::info("[{}] '{}' = {} (ZMSG)", i, sEntName, iValue);
                        break;
                    case Glacier::CCom::Ent::TypeInfo::EKind::SREF:
                        spdlog::info("[{}] '{}' = {:X} (SREF?)", i, sEntName, iValue);
                        break;
                    default:
                        spdlog::info("[{}] '{}' = <Bad type info> {:8X}, got type mask: '{:4X}'", i, sEntName, reinterpret_cast<std::intptr_t>(pEnt), pEnt->m_pTypeInfo->m_eKind);
                        break;
                }
            }
        }
        ImGui::End();
    }

    bool SandboxWidget::g_bIsVisible = false;
}