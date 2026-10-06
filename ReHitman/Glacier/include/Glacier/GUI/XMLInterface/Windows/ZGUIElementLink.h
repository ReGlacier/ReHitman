#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
    // fwds
    class ZCONTROL;

    struct ZGUIElementLink
    {
        // members
        float m_fSizeX;
        float m_fSizeY;
        ZCONTROL* m_apFocusIn[4];
        bool m_bUseLinks;

        // methods
        ZGUIElementLink();
        ZGUIElementLink(float fSizeX, float fSizeY);
        ZGUIElementLink(float fSizeX, float fSizeY, ZCONTROL* pFocus);
        ZGUIElementLink(float fSizeX, float fSizeY, ZCONTROL* pFocus0, ZCONTROL* pFocus1, ZCONTROL* pFocus2, ZCONTROL* pFocus3);

        ZGUIElementLink& operator=(const ZGUIElementLink& other);
        bool UseLinks() const;
    };
    RE_VERIFY_SIZE(ZGUIElementLink, 0x1C);
    RE_VERIFY_OFFSET(ZGUIElementLink, m_fSizeX, 0x00);
    RE_VERIFY_OFFSET(ZGUIElementLink, m_fSizeY, 0x04);
    RE_VERIFY_OFFSET(ZGUIElementLink, m_apFocusIn, 0x08);
    RE_VERIFY_OFFSET(ZGUIElementLink, m_bUseLinks, 0x18);
}
