#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZFRAME;

    class ZXMLFrame : public IGUIElement
    {
    public:
        // vtbl
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;

        // methods
        ZXMLFrame();
        ~ZXMLFrame();

        // members
        ZVector2 m_v2Size; // +0x68
        ZFRAME* m_pFrame;  // +0x70
        zstring m_sFrame;  // +0x74
    };
    RE_VERIFY_SIZE(ZXMLFrame, 0x80);
    RE_VERIFY_OFFSET(ZXMLFrame, m_v2Size, 0x68);
    RE_VERIFY_OFFSET(ZXMLFrame, m_pFrame, 0x70);
    RE_VERIFY_OFFSET(ZXMLFrame, m_sFrame, 0x74);
}
