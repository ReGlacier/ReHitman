#pragma once

#include <cstdint>
#include <Glacier/ReGlacier.h>
#include <Glacier/Render/PFORMAT.h>
#include <BloodMoney/Game/LoaderSequence/ZLoader_Sequence_Player_Base.h>


namespace Hitman::BloodMoney
{
    struct ZLoader_Sequence_Script_Reader;

    struct ZGeometrySubObject
    {
        uint16_t m_usSize[2];
        float m_fOffset[2];
        float m_fMoveSpeed[2];
        char m_szName[24];
        int *pTexture;
        uint32_t *m_pAnimData;
        Glacier::ZPFORMAT m_Format;
        uint32_t m_dwDataSize;
        float m_fScaleFactor;
        uint16_t m_lTextureId;
        uint16_t m_lDrawEntryId;
        float m_fCustomParamX;
        float m_fCustomParamY;
        int *pVertexBuffer;
    };
    RE_VERIFY_SIZE(ZGeometrySubObject, 0x50);

    struct ZGeometryObjectHeader
    {
        ZGeometrySubObject *pSubObjectsArray;
        int iSubObjectCount;
    };
    RE_VERIFY_SIZE(ZGeometryObjectHeader, 0x8);


    class ZLoader_Sequence_Wintel_D3D : public ZLoader_Sequence_Player_Base
    {
    public:
        // vtbl (no changes)
        // members
        uint32_t m_hEvent;  // +0x4
        uint32_t m_hThread; // +0x8
        ZLoader_Sequence_Script_Reader *pReader; // +0xC
        int m_pPoolEnd; // +0x10
        float m_fCurrentTime; // +0x14
        uint32_t m_field18;
        uint32_t m_field1C;
        ZGeometryObjectHeader* m_aObjects; // +0x20
        int m_iTotalObjects;
        int m_pUnknownPool;
        int m_iUnknownPoolFreeObjects;
    };
    RE_VERIFY_SIZE(ZLoader_Sequence_Wintel_D3D, 0x30);
    RE_VERIFY_OFFSET(ZLoader_Sequence_Wintel_D3D, m_fCurrentTime, 0x14);
    RE_VERIFY_OFFSET(ZLoader_Sequence_Wintel_D3D, m_aObjects, 0x20);
}