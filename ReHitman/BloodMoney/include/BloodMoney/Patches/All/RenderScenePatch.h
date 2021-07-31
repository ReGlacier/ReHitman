#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class RenderScenePatch final : public BasicPatch
    {
        HF::Hook::TrampolinePtr<6> m_patch;
    public:
        RenderScenePatch() = default;

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}