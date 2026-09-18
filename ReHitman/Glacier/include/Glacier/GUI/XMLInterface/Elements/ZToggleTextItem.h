#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    class ZToggleTextItem : public ZGUIBase
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;

        // methods
        ZToggleTextItem();
        ~ZToggleTextItem();

        const char* GetText() const;

        // members
        zstring m_sText; // +0x4c
    };
    RE_VERIFY_SIZE(ZToggleTextItem, 0x58);
    RE_VERIFY_OFFSET(ZToggleTextItem, m_sText, 0x4c);
}
