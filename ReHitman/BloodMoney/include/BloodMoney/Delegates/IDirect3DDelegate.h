#pragma once

#include <d3d9.h>

namespace Hitman::BloodMoney
{
    class IDirect3DDelegate
    {
    public:
        virtual ~IDirect3DDelegate() noexcept = default;

        virtual void OnInitialised(IDirect3DDevice9* device) = 0;
        virtual void OnBeginScene(IDirect3DDevice9* device) = 0;
        virtual void OnEndScene(IDirect3DDevice9* device) = 0;
        virtual void OnDeviceLost() = 0;
        virtual void OnReset(IDirect3DDevice9* device) = 0;
        virtual void OnDeviceRestored(IDirect3DDevice9* device) = 0;
    };
}