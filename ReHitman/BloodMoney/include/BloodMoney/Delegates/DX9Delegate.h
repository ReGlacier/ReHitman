#pragma once

#include <BloodMoney/Delegates/IDirect3DDelegate.h>
#include <BloodMoney/Debug/GizmosControl.h>

namespace Hitman::BloodMoney
{
    class DX9Delegate final : public IDirect3DDelegate, public Debug::GizmosControl
    {
    public:
        void OnInitialised(IDirect3DDevice9* device) override;
        void OnDeviceLost() override;
        void OnReset(IDirect3DDevice9* device) override;
        void OnDeviceRestored(IDirect3DDevice9* device) override;
        void OnPresent(IDirect3DDevice9* device) override;

    private:
        void DrawDebugUI(IDirect3DDevice9* device);
    };
}