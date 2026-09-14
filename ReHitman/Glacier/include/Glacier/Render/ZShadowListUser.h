#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZBaseGeom;

    struct SetHeader
    {
        const ZBaseGeom* pLight;
        SetHeader* pNext;
        int lNumCasters;
    };
    RE_VERIFY_SIZE(SetHeader, 0xC);

    struct CasterHeader
    {
        const ZBaseGeom* pCaster;
        int lNumReceivers;
    };
    RE_VERIFY_SIZE(CasterHeader, 0x8);

    class ZShadowListUser
    {
    public:
        // methods
        void Init(uint8_t* pxData);
        int GetNumSets() const;
        SetHeader* GetSetHeader();
        CasterHeader* GetCasterHeader();
        const ZBaseGeom** GetReceiverBase(int nReceivers);

        // members
        uint8_t* m_pxData;
        uint8_t* m_pxWork;
    };
}
