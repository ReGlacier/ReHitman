#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class IInputDelegate;

    class InputDevicesPatches final : public BasicPatch
    {
        static constexpr size_t kRegisterClassExPatchSize = 6;

        HF::Hook::TrampolineBasicPtr m_wintelMouseCtorHook;
        HF::Hook::TrampolinePtr<kRegisterClassExPatchSize> m_registerClassExHook;

    public:
        InputDevicesPatches() = default;
        explicit InputDevicesPatches(std::unique_ptr<IInputDelegate>&& delegate);

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}