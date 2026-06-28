#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    struct FixFuckedLoaderScreens : public BasicPatch
    {
        std::string_view GetName() const override { return "FixFuckedLoaderScreen"; }
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;

    private:
        HF::Hook::TrampolineBasicPtr m_correctTimeCalcTrampoline;
        HF::Hook::TrampolineBasicPtr m_newWaitLoopTrampoline;
    };
}