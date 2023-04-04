#include <BloodMoney/Patches/Mods/SkinChanger.h>

#include <BloodMoney/Patches/All/ZHitman3Patches.h>
#include <BloodMoney/Game/Globals.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3Actor.h>

#include <Glacier/Glacier.h>
#include <Glacier/ZAction.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/EventBase/ZEventBuffer.h>

#include <spdlog/spdlog.h>

#define CAPTURE_THIS(type, varname) \
    static type* varname = { nullptr }; \
    __asm { mov varname, ecx }


namespace Hitman::BloodMoney
{
    namespace Callbacks
    {

        std::intptr_t g_ClassCommand      = 0;
        std::intptr_t g_EnableClothPickup = 0x0063F890;
        static const Glacier::ZMSGID g_ChangeSuitMSGID = 0x512;

        int __stdcall OnClassCommand(Glacier::ZMSGID msg, void* arg) {
            CAPTURE_THIS(Glacier::ZGEOM, pSelf)

            auto gameData = Glacier::getInterface<Hitman::BloodMoney::ZHM3GameData>(Hitman::BloodMoney::Globals::kGameDataAddr);
            if (gameData && msg == g_ChangeSuitMSGID) {
                auto playerGeom = reinterpret_cast<Glacier::ZGEOM*>(gameData->m_Hitman3);

                reinterpret_cast<Glacier::ZLNKOBJ*>(playerGeom)->CopyGeometryFrom(pSelf->m_baseGeom->m_primitive);
                reinterpret_cast<Glacier::ZLNKOBJ*>(playerGeom)->UpdateGeometry(true);

                ((void(__cdecl*)(Glacier::ZGEOM*, const char*))0x005D3BA0)(pSelf, "Take clothes"); //RemoveCUIAction
                return 0;
            }

            return ((int(__thiscall*)(Glacier::ZGEOM*, Glacier::ZMSGID, void*))g_ClassCommand)(pSelf, msg, arg);
        }

        void __stdcall OnPrepareActorBody() {
            CAPTURE_THIS(Hitman::BloodMoney::ZHM3Actor, pSelf)
            // --- Mod logic ---
            auto sysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
            if (!sysInterface) { return; }

            auto engineDb = sysInterface->m_engineDataBase;
            if (!engineDb) { return; }

            auto listener = reinterpret_cast<Glacier::ZGEOM*>(pSelf);
            if (!listener) { return; }

            if (!g_ClassCommand) {
                g_ClassCommand = HF::Hook::VFHook<Glacier::ZGEOM>::makeHook(listener, 89, &Callbacks::OnClassCommand);
            }

            Glacier::ZREF ref = listener->GetRef();

            auto actionId = Glacier::ZAction::AddAction(reinterpret_cast<Glacier::ZGEOM*>(pSelf),
                                        "Take clothes",
                                        "Take clothes",
                                        Glacier::EActionType::CUSTOM,
                                        g_ChangeSuitMSGID,
                                        ref,
                                        0,
                                        270);

            auto pAction = Glacier::ZEventBuffer::EventRefToInstance<Glacier::ZAction>(actionId);
            if (pAction) { // It's not required but why not
                pAction->Show();
            }

            ((void(__thiscall*)(Hitman::BloodMoney::ZHM3Actor*))g_EnableClothPickup)(pSelf);
        }
    }

    std::string_view SkinChanger::GetName() const { return "Mods/SkinChanger"; }

    bool SkinChanger::Apply(const ModPack& modules)
    {
        HF::Win32::VProtect protection { 0x007969F4, sizeof(uint32_t), PAGE_READWRITE };
        int* ppTableAddr = (int*)0x007969F4;
        auto x = &Callbacks::OnPrepareActorBody;
        *ppTableAddr = (reinterpret_cast<uint32_t>(reinterpret_cast<uint32_t*&>(x)));

        // On OK
        BasicPatch::Apply(modules);
        return true;
    }

    void SkinChanger::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);
    }
}