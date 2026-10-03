#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZWINPIC;

    class ZGraphic : public IGUIElement
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;

        // methods
        ZGraphic();
        ~ZGraphic();

        void SetGraphic(const char* pGraphic);

        // members
        zstring m_sGraphic;         // +0x68
        ZWINGROUP* m_pGraphicGroup; // +0x74
        bool m_bMirrorY;            // +0x78
        int32_t m_iPriority;        // +0x7c
        bool m_bUseAsButtons;       // +0x80
        bool m_bAnimateAlpha;       // +0x81

    protected:
        void DoMirror();
    };
    RE_VERIFY_SIZE(ZGraphic, 0x84);
    RE_VERIFY_OFFSET(ZGraphic, m_sGraphic, 0x68);
    RE_VERIFY_OFFSET(ZGraphic, m_pGraphicGroup, 0x74);
    RE_VERIFY_OFFSET(ZGraphic, m_bMirrorY, 0x78);
    RE_VERIFY_OFFSET(ZGraphic, m_iPriority, 0x7c);
    RE_VERIFY_OFFSET(ZGraphic, m_bUseAsButtons, 0x80);
    RE_VERIFY_OFFSET(ZGraphic, m_bAnimateAlpha, 0x81);
}
