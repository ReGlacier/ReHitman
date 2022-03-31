#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class IInputDelegate;

    class InputDevicesPatches final : public BasicPatch
    {
    public:
        InputDevicesPatches() = default;
        explicit InputDevicesPatches(std::unique_ptr<IInputDelegate>&& delegate);

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}