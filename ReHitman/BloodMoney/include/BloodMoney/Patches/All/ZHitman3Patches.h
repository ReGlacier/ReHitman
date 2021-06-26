#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class ZHitman3Patches final : public BasicPatch
    {
    private:
        HF::Hook::TrampolineBasicPtr m_constructor;

    public:
        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}