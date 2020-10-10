#include <BloodMoney/Patches/All/ZHitman3Patches.h>
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
    }

    namespace Callbacks
    {
        void __stdcall OnZHitman3Constructed(std::intptr_t instance)
        {
            auto pSysInterface = Glacier::getInterface<Glacier::ZSysInterfaceWintel>(Globals::kSysInterfaceAddr);
            spdlog::info("Current scene: {}", pSysInterface->m_engineDataBase->GetSceneName());
            spdlog::info("ZHitman3 constructed at {:08X}", instance);
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
             * @brief Setup another patch?
             */

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
        m_constructor->remove();
        if (auto process = modules.process.lock())
        {
            HF::Hook::MoveInstructions<4>(process, Consts::kZHitman3Ctor + 5, Consts::kZHitman3Ctor);
            HF::Hook::FillMemoryByNOPs(process, Consts::kZHitman3Ctor + 5, 5);
        }
    }
}

#undef ENABLE_MODULE