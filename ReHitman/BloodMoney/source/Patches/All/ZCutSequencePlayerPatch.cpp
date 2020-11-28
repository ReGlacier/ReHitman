#include <BloodMoney/Patches/All/ZCutSequencePlayerPatch.h>
#include <BloodMoney/Editors/CutSequencesPool.h>
#include <BloodMoney/Game/ZCutSequencePlayer.h>

#include <spdlog/spdlog.h>
#include <memory>

namespace Consts
{
    static constexpr std::intptr_t kConstructorPatchAddr = 0x00544BB5;
    static constexpr std::intptr_t kDestructorPatchAddr  = 0x00544D60;
}

namespace Callbacks
{
    static void __stdcall OnZCutSequencePlayerConstructed(Hitman::BloodMoney::ZCutSequencePlayer* player)
    {
        spdlog::info("ZCutSequencePlayer constructed at {:8X}", ((uint32_t)player));
        Hitman::BloodMoney::CutSequencesPool::GetInstance().Add(player);
    }

    static void __stdcall OnZCutSequencePlayerDestructed(Hitman::BloodMoney::ZCutSequencePlayer* player)
    {
        spdlog::info("ZCutSequencePlayer destroyed at {:8X}", ((uint32_t)player));
        Hitman::BloodMoney::CutSequencesPool::GetInstance().Remove(player);
    }
}

namespace Hitman::BloodMoney
{
    std::string_view ZCutSequencePlayerPatch::GetName() const { return "ZCutSequencePlayer creation patch"; }

    bool ZCutSequencePlayerPatch::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            m_constructorTrampoline = HF::Hook::HookFunction<void(__stdcall*)(ZCutSequencePlayer*), 8>(
                    process,
                    Consts::kConstructorPatchAddr,
                    &Callbacks::OnZCutSequencePlayerConstructed,
                    {
                        HF::X86::PUSH_AD,
                        HF::X86::PUSH_FD,
                        HF::X86::PUSH_ECX
                    },
                    {
                        HF::X86::POP_FD,
                        HF::X86::POP_AD,
                    });
            if (!m_constructorTrampoline->setup())
            {
                spdlog::error("Failed to apply patch for ZCutSequencePlayerPatch (ctor)");
                return false;
            }

            m_destructorTrampoline = HF::Hook::HookFunction<void(__stdcall*)(ZCutSequencePlayer*), 6>(
                    process,
                    Consts::kDestructorPatchAddr,
                    &Callbacks::OnZCutSequencePlayerDestructed,
                    {
                        HF::X86::PUSH_AD,
                        HF::X86::PUSH_FD,
                        HF::X86::PUSH_ECX
                    },
                    {
                        HF::X86::POP_FD,
                        HF::X86::POP_AD,
                    });
            if (!m_destructorTrampoline->setup())
            {
                spdlog::error("Failed to apply patch for ZCutSequencePlayerPatch (dtor)");
                m_constructorTrampoline->remove(); // reject first patch
                return false;
            }

            return BasicPatch::Apply(modules);
        }
        return false;
    }

    void ZCutSequencePlayerPatch::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        m_constructorTrampoline->remove();
        m_constructorTrampoline = nullptr;

        m_destructorTrampoline->remove();
        m_destructorTrampoline = nullptr;
    }
}