#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZShadowListUser.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <cstdint>


namespace Glacier
{
    class ZBaseGeom;
    class CListUser;
    class ZRenderEntryGeom;

    class ZShadowListBuild
    {
    public:
        // methods
        void Init(uint8_t* pxCompiled, int lBufferSize);
        int NewSetBegin(const ZBaseGeom* pLight);
        void NewSetEnd();
        int NewCasterBegin(const ZBaseGeom* pCaster);
        void RegisterReceiver(const ZBaseGeom* pReceiver);
        void Compile();
        void AddReceiver(const ZBaseGeom* pReceiver);
        void AddCaster(const ZBaseGeom* pCaster);
        bool ValidateCaster(const ZBaseGeom* pCaster) const;
        bool ValidateReceiver(const ZBaseGeom* pReceiver) const;
        void ProcessEnvironment(ZStackArray<512, ZRenderEntryGeom*>* pLnkList);
        void ProcessLight(CListUser* pListUser, ZBaseGeom* pLightBaseGeom);

        // members
        const ZBaseGeom* m_paReceivers[256];
        const ZBaseGeom* m_paCasters[128];
        uint8_t* m_pxCompiled;
        const ZBaseGeom** m_pRecStore;
        SetHeader* m_pSetHeader;
        CasterHeader* m_pCasterHeader;
        int m_lNumSets;
        int m_lNumCasters;
        int m_lNumReceivers;
        uint8_t* m_pxStore;
        uint8_t* m_pxEnd;
    };
    RE_VERIFY_SIZE(ZShadowListBuild, 0x624); // Verified PC BuildDrawChainDropShadows stack layout
}
