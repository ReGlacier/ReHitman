#include <BloodMoney/Patches/All/RenderScenePatch.h>
#include <BloodMoney/Delegates/RenderDelegateManager.h>
#include <Glacier/ZCAMERA.h>
#include <spdlog/spdlog.h>

#define CAPTURE_THIS(type, varname) \
    static type* varname = { nullptr }; \
    __asm { mov varname, ecx }

#include <BloodMoney/Game/UI/ZLINEOBJ.h>

namespace Hitman::BloodMoney {
    namespace Callbacks
    {
        static void __stdcall OnDrawBufferViewUpdate(Glacier::ZDrawBuffer* pDrawBuffer, Glacier::ZCameraSpace* pCameraSpace) {
            CAPTURE_THIS(int, savedThis)

            // Invoke original function #56 of savedThis thing
            using OriginalType = int;
            HF::Hook::VFHook<OriginalType>::invoke<void, Glacier::ZDrawBuffer*, Glacier::ZCameraSpace*>(
                    reinterpret_cast<std::intptr_t*>(savedThis),
                    56,
                    pDrawBuffer,
                    pCameraSpace);

            // Custom things
            static ZLINEOBJ* pCustomLine = nullptr;
            if (!pCustomLine) {
                using Allocate_t = ZLINEOBJ*(__cdecl*)();
                auto allocateLineObj = (Allocate_t)0x005558E0;

                pCustomLine = allocateLineObj();
                spdlog::info("Allocated new ZLINEOBJ at {:08X}", ((int)pCustomLine));

                ZFONT* pFont = reinterpret_cast<ZFONT*>(Glacier::ZGEOM::RefToPtr(0x3EB));
                pCustomLine->SetFont(pFont);

                pCustomLine->SetWidth(pCustomLine->GetStringWidth("Hello world from ReHitman!", 26));
                pCustomLine->SetText("Hello world from ReHitman!");

                pCustomLine->SetPos(200.f, 25.f, 0.f);
                pCustomLine->SetColor(0xFFFF0000);

                Glacier::ZVector2 sv;
                sv.x = -1.0f;
                sv.y = 1.0f;
                pCustomLine->SetScale(&sv, true);
            }

            pCustomLine->DrawBufferViewUpdate(pDrawBuffer, pCameraSpace);
        }
    }

    std::string_view RenderScenePatch::GetName() const {
        return "Scene renderer patch";
    }

    bool RenderScenePatch::Apply(const ModPack& modules) {
        if (auto process = modules.process.lock())
        {
            //TODO: Write 3 hooks to invoke our delegates
            process->fillMemory(0x004B9FBA, HF::X86::NOP, 6);

            m_patch = HF::Hook::HookFunction<void(__stdcall*)(Glacier::ZDrawBuffer*, Glacier::ZCameraSpace*), 6>(
                    process,
                    0x004B9FBA,
                    &Callbacks::OnDrawBufferViewUpdate,
                    {
                    },
                    {
                    });

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