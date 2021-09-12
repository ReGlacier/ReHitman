#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class GlacierHooksPatch final : public BasicPatch
    {
        HF::Hook::TrampolineBasicPtr m_onUpdateTrampoline;
    public:
        GlacierHooksPatch() = default;

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}