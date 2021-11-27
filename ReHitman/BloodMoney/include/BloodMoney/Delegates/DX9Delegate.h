#pragma once

#include <BloodMoney/Delegates/IDirect3DDelegate.h>

namespace Hitman::BloodMoney
{
    class DX9Delegate final : public IDirect3DDelegate
    {
    public:
        void OnInitialised(IDirect3DDevice9* device) override;
        void OnDeviceLost() override;
        void OnReset(IDirect3DDevice9* device) override;
        void OnDeviceRestored(IDirect3DDevice9* device) override;
        void OnPresent(IDirect3DDevice9* device) override;

    private:
        void DrawGizmos();
        void DrawDebugUI(IDirect3DDevice9* device);
    };
}