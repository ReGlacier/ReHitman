#include <BloodMoney/Patches/All/GlacierHooksPatch.h>
#include <BloodMoney/Client.h>
#include <ScriptingRuntime.h>

#include <spdlog/spdlog.h>

#include <string_view>
#include <array>

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        static constexpr std::intptr_t kMainLoopAddr = 0x0045D8C4;
    }

    namespace Callbacks
    {
        static void __stdcall ZSysInterfaceWintel_MainLoop() {
            constexpr float kFixedTimeDelta = 1.0f / 60.f; //TODO: Locate where actual time delta placed in ZSysInterfaceWintel class
            if (auto pRuntime = Client::GetScriptingRuntime()) {
                pRuntime->OnUpdate(kFixedTimeDelta);
            }
        }
    }

    std::string_view GlacierHooksPatch::GetName() const { return "GlacierHooksPatch Patch"; }

    bool GlacierHooksPatch::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            process->fillMemory(Consts::kMainLoopAddr, HF::X86::NOP, 26);

            m_onUpdateTrampoline = HF::Hook::HookFunction(
                    process,
                    Consts::kMainLoopAddr,
                    &Callbacks::ZSysInterfaceWintel_MainLoop,
                    {
                        HF::X86::PUSH_AD,
                        HF::X86::PUSH_FD
                    },
                    {
                        HF::X86::POP_FD,
                        HF::X86::POP_AD
                    });

            if (!m_onUpdateTrampoline->setup()) {
                spdlog::error("Failed to apply patch for GlacierHooksPatch (MainLoop)");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void GlacierHooksPatch::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        if (auto process = modules.process.lock()) {
            process->fillMemory(Consts::kMainLoopAddr, HF::X86::NOP, 26);
        }

        m_onUpdateTrampoline->remove();
        m_onUpdateTrampoline = nullptr;
    }
}