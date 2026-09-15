#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRD3DDynamicVB
    {
    public:
        // methods
        ZRD3DDynamicVB(ZDirect3DDevice* pDevice, uint32_t lLength, uint32_t lUsage, D3DPOOL ePool);
        ~ZRD3DDynamicVB();

        IDirect3DVertexBuffer9* Interface();
        void* Lock(uint32_t lNumVertices, uint32_t lVertexSize, uint32_t& rOutStartVertex);
        void Unlock();

        // members
        IDirect3DVertexBuffer9* m_pVertexBuffer;
        void* m_pLockedData;                        // non-null while the buffer is locked
        uint32_t m_lLength;
        uint32_t m_lCurrentOffset;                  // ring-buffer write position in bytes
        bool m_bForceReset;                         // set externally; next Lock rewinds to 0 with D3DLOCK_DISCARD
    };
    RE_VERIFY_SIZE(ZRD3DDynamicVB, 0x14); // Verified PC allocation
}
