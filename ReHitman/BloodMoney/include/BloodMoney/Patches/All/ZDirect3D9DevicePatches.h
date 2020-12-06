#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class IDirect3DDelegate;

    class ZDirect3D9DevicePatches final : public BasicPatch
    {
    private:
        HF::Hook::TrampolinePtr<10> m_ZDirect3DDevice_Constructor { nullptr };
    public:
        explicit ZDirect3D9DevicePatches(std::unique_ptr<IDirect3DDelegate>&& delegate);

        [[nodiscard]] std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };

}