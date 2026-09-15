#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRD3DDynamicIB32
    {
    public:
        // methods
        ZRD3DDynamicIB32(ZDirect3DDevice* pDevice, uint32_t lLength, uint32_t lUsage, D3DPOOL ePool);
        ~ZRD3DDynamicIB32();

        IDirect3DIndexBuffer9* Interface();
        void* Lock(uint32_t lNumIndices, uint32_t& rOutStartIndex);
        void Unlock();

        // members
        IDirect3DIndexBuffer9* m_pIndexBuffer;
        void* m_pLockedData;
        uint32_t m_lLength;
        uint32_t m_lCurrentOffset;
        bool m_bForceReset; // set externally; next Lock rewinds to 0 with D3DLOCK_DISCARD
    };
    RE_VERIFY_SIZE(ZRD3DDynamicIB32, 0x14); // Verified PC allocation
}
