#include <BloodMoney/Patches/All/RenderScenePatch.h>
#include <BloodMoney/Delegates/RenderDelegateManager.h>
#include <BloodMoney/Game/Globals.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/ZRenderEntry.h>
#include <Glacier/ZCAMERA.h>
#include <spdlog/spdlog.h>

#define CAPTURE_THIS(type, varname) \
    static type* varname = { nullptr }; \
    __asm { mov varname, ecx }

#include <BloodMoney/Game/UI/ZLINEOBJ.h>

namespace Hitman::BloodMoney {
    namespace Callbacks
    {
        static void __stdcall OnDrawBufferViewUpdate(
                Glacier::ZRenderEntry* pRenderCameraEntry,
                Glacier::ZDrawBuffer* pDrawBuffer,
                Glacier::ZCameraSpace* pCameraSpace,
                Glacier::ZRenderEntry* pRenderEntry
        )
        {
            if (!pDrawBuffer || !pCameraSpace || !pRenderEntry || !pRenderCameraEntry) {
                return;
            }

            Glacier::ZEntityLocator* pEntityLocator = pRenderEntry->GetEntityLocator();
            if (!pEntityLocator) {
                return;
            }

            auto pGeom = reinterpret_cast<Glacier::ZGEOM*>(pEntityLocator->m_assignedTo);
            if (!pGeom) {
                return;
            }

            spdlog::info("CAMERA: {}", pRenderCameraEntry->GetEntityLocator()->entityName);

            using OriginalType = int;
            HF::Hook::VFHook<OriginalType>::invoke<void, Glacier::ZDrawBuffer*, Glacier::ZCameraSpace*>(
                    reinterpret_cast<std::intptr_t*>(pGeom),
                    56,
                    pDrawBuffer,
                    pCameraSpace);
        }
    }

    std::string_view RenderScenePatch::GetName() const {
        return "Scene renderer patch";
    }

    bool RenderScenePatch::Apply(const ModPack& modules) {
        if (auto process = modules.process.lock())
        {
            process->fillMemory(0x004B9F9F, HF::X86::NOP, 33);

            m_patch = HF::Hook::HookFunction<void(__stdcall*)(Glacier::ZRenderEntry*, Glacier::ZDrawBuffer*, Glacier::ZCameraSpace*, Glacier::ZRenderEntry*), 6>(
                    process,
                    0x004B9FBA,
                    &Callbacks::OnDrawBufferViewUpdate,
                    {
                        0x8B, 0x4C, 0x24, 0x24,                     //mov    ecx,DWORD PTR [esp+0x24]
                        0x51,                                       //push   ecx ; pointer to current entry to draw
                        0x8D, 0x94, 0x24, 0xD0, 0x01, 0x00, 0x00,   //lea    edx,[esp+0x1cc+4]
                        0x52,                                       //push   edx ; pointer to camera space
                        0x8D, 0x54, 0x24, 0x30,                     //lea    edx,[esp+0x2c+4]
                        0x52,                                       //push   edx ; pointer to draw buffer
                        0x56                                        //push   esi ; pointer to render camera entry
                    },
                    {});

            if (!m_patch->setup())
            {
                spdlog::error("Failed to apply patch for RenderScenePatch (renderer of something)");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void RenderScenePatch::Revert(const ModPack& modules) {
        BasicPatch::Revert(modules);

        m_patch->remove();
    }
}