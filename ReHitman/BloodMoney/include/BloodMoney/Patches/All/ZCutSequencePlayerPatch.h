#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class ZCutSequencePlayerPatch final : public BasicPatch
    {
        HF::Hook::TrampolinePtr<8> m_constructorTrampoline;
        HF::Hook::TrampolinePtr<6> m_destructorTrampoline;
    public:
        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}