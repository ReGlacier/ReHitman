#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    class ZTextBlock : public IGUIElement
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual void SetText();

        // methods
        ZTextBlock();
        ~ZTextBlock();

        // members
        ZWINGROUP* m_pWinGroupText; // +0x68
        int32_t m_iLineSpace;       // +0x6c
        zstring m_sText;            // +0x70
        int32_t m_iWidth;           // +0x7c
    };
    RE_VERIFY_SIZE(ZTextBlock, 0x80);
    RE_VERIFY_OFFSET(ZTextBlock, m_pWinGroupText, 0x68);
    RE_VERIFY_OFFSET(ZTextBlock, m_iLineSpace, 0x6c);
    RE_VERIFY_OFFSET(ZTextBlock, m_sText, 0x70);
    RE_VERIFY_OFFSET(ZTextBlock, m_iWidth, 0x7c);

    class ZTextBlockSpeech : public ZTextBlock
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual void SetText() override;

        // methods
        ZTextBlockSpeech();
        ~ZTextBlockSpeech();

        // members
        uint32_t m_iSoundIdLocale; // +0x80
        uint32_t m_iSoundRef;      // +0x84
        bool m_bShowText;          // +0x88
    };
    RE_VERIFY_SIZE(ZTextBlockSpeech, 0x8c);
    RE_VERIFY_OFFSET(ZTextBlockSpeech, m_iSoundIdLocale, 0x80);
    RE_VERIFY_OFFSET(ZTextBlockSpeech, m_iSoundRef, 0x84);
    RE_VERIFY_OFFSET(ZTextBlockSpeech, m_bShowText, 0x88);
}
