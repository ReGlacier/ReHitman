#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class ZHitman3Patches final : public BasicPatch
    {
    private:
        HF::Hook::TrampolineBasicPtr m_constructor;
        HF::Hook::TrampolineBasicPtr m_guardControlCtor;
        HF::Hook::TrampolineBasicPtr m_guardControlDtor;

    public:
        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}