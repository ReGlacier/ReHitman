#include <BloodMoney/Patches/All/ZHitman3Patches.h>
#include <BloodMoney/Game/ZGuardQuarterController.h>
#include <BloodMoney/Game/Globals.h>

#include <Glacier/Glacier.h>
#include <Glacier/ZSysInterfaceWintel.h>
#include <Glacier/ZEngineDataBase.h>

#include <spdlog/spdlog.h>

#define ENABLE_MODULE(mod, modName) \
    do {                            \
        if (!mod->setup()) {        \
            spdlog::error("Failed to initialise module {}", modName); \
        } else { \
            spdlog::info("Patch {} inited at {:08X}", modName, mod->at()); \
        } \
    } while(0);

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        static constexpr std::intptr_t kZHitman3Ctor = 0x00604CF1;

        static constexpr std::intptr_t kZGuardQuarterControllerCtor = 0x00657BC5;
        static constexpr std::intptr_t kZGuardQuarterControllerDtor = 0x00657349;
    }

    namespace Callbacks
    {
        void __stdcall OnZHitman3Constructed(std::intptr_t instance)
        {
            auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
            spdlog::info("Current scene: {}", pSysInterface->m_engineDataBase->GetSceneName());
            spdlog::info("ZHitman3 constructed at {:08X}", instance);
        }

        void __stdcall OnZGuardQuarterControllerConstructed(ZGuardQuarterController* pInstance) {
            spdlog::info("ZGuardQuarterController inited at {:08X}", reinterpret_cast<std::intptr_t>(pInstance));
            ZGuardQuarterController::g_pCurrentLevelGuardControl = pInstance;
        }

        void __stdcall OnZGuardQuarterControllerDestroyed() {
            spdlog::info("ZGuardQuarterController destroyed");
            ZGuardQuarterController::g_pCurrentLevelGuardControl = nullptr;
        }
    }

    std::string_view ZHitman3Patches::GetName() const
    {
        return "ZHitman3 (Common)";
    }

    bool ZHitman3Patches::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            /**
             * @brief Setup patch for ZHitman3::Ctor
             */
            HF::Hook::MoveInstructions<4>(process, Consts::kZHitman3Ctor, Consts::kZHitman3Ctor + 5);
            HF::Hook::FillMemoryByNOPs(process, Consts::kZHitman3Ctor, 5);
            m_constructor = HF::Hook::HookFunction(
                    process, Consts::kZHitman3Ctor, &Callbacks::OnZHitman3Constructed,
                    { HF::X86::PUSH_ECX, HF::X86::PUSH_EAX, HF::X86::PUSH_EAX },
                    { HF::X86::POP_EAX, HF::X86::POP_EAX });
            ENABLE_MODULE(m_constructor, "ZHitman3::Ctor")

            /**
             * @brief Setup hook for ZGuardQuarterController creation & destroy
             */
            HF::Hook::FillMemoryByNOPs(process, Consts::kZGuardQuarterControllerCtor, 6);
            m_guardControlCtor = HF::Hook::HookFunction(process, Consts::kZGuardQuarterControllerCtor, &Callbacks::OnZGuardQuarterControllerConstructed,
                { HF::X86::PUSH_AD, HF::X86::PUSH_FD, HF::X86::PUSH_EAX }, { HF::X86::POP_FD, HF::X86::POP_AD, HF::X86::RETN });
            ENABLE_MODULE(m_guardControlCtor, "ZGuardQuarterController::Ctor");

            HF::Hook::FillMemoryByNOPs(process, Consts::kZGuardQuarterControllerDtor, 6);
            m_guardControlDtor = HF::Hook::HookFunction(process, Consts::kZGuardQuarterControllerDtor, &Callbacks::OnZGuardQuarterControllerDestroyed, {}, { 0xC2, 0x04, 0x00 });
            ENABLE_MODULE(m_guardControlDtor, "ZGuardQuarterController::Dtor");

            // On OK
            BasicPatch::Apply(modules);
            return true;
        }

        return false;
    }

    void ZHitman3Patches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);
        // Revert changes
        if (auto process = modules.process.lock())
        {
            m_constructor->remove();

            HF::Hook::MoveInstructions<4>(process, Consts::kZHitman3Ctor + 5, Consts::kZHitman3Ctor);
            HF::Hook::FillMemoryByNOPs(process, Consts::kZHitman3Ctor + 5, 5);
        }
    }
}

#undef ENABLE_MODULE