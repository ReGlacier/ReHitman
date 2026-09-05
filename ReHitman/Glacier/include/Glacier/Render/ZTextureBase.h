#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/PFORMAT.h>
#include <cstdint>


namespace Glacier
{
    class ZTextureBase
    {
    public:
        // constants
        static constexpr int TEXNAMELENGTH = 0x18;
        // vtbl

        // methods
        ZTextureBase();

        void SetName(const char* pszName);

        // members
        uint16_t m_usSize[2] { 0 };
        float m_fOffset[2] { 0.f };
        float m_fMoveSpeed[2] { 0.f };
        char m_szName[TEXNAMELENGTH] { '\0' };
        void* m_pUserData { nullptr };
        uint32_t* m_pAnimData { nullptr };
        ZPFORMAT m_Format { ZPFORMAT::ZPF_FORCE_LONG };
        uint32_t m_dwDataSize { 0u };
        float m_fScaleFactor { 1.f };
        uint16_t m_lTextureId { 0u };
        uint16_t m_lDrawEntryId { 0u };
    };
    RE_VERIFY_SIZE(ZTextureBase, 0x44);
    RE_VERIFY_OFFSET(ZTextureBase, m_pAnimData, 0x30); // Verified PC ZRenderMaterialBinderParser::CreatePropertyBinders
}