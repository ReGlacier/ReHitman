#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRD3DStaticIB
    {
    public:
        // methods
        ZRD3DStaticIB(ZDirect3DDevice* pDevice, uint32_t lLength, uint32_t lUsage, D3DPOOL ePool);
        ~ZRD3DStaticIB();

        IDirect3DIndexBuffer9* Interface();
        void* LockedData();

        // members
        IDirect3DIndexBuffer9* m_pIndexBuffer;
        void* m_pLockedData;
        uint32_t m_lLength;
    };
    RE_VERIFY_SIZE(ZRD3DStaticIB, 0xC);
}