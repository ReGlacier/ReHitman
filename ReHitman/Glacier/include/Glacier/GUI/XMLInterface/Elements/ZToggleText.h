#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggle.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZSTL/zvector.h>


namespace Glacier
{
    // fwds
    class ZToggleTextItem;

    class ZToggleText : public ZToggle
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual void OptionChanged() override;
        virtual void AddText(const char* pText);

        // methods
        ZToggleText();
        ~ZToggleText();

        void AddText(const char* pText, uint32_t dwLen);

        // members
        bool m_bSingleResource;                 // +0xa4
        bool m_bFirstTimeInit;                  // +0xa5
        zvector<zstring*> m_vecTextPtr;         // +0xa8
        zvector<zstring*> m_vecResourceTextPtr; // +0xb4

    protected:
        void ParseResourceText();
    };
    RE_VERIFY_SIZE(ZToggleText, 0xc0);
    RE_VERIFY_OFFSET(ZToggleText, m_bSingleResource, 0xa4);
    RE_VERIFY_OFFSET(ZToggleText, m_bFirstTimeInit, 0xa5);
    RE_VERIFY_OFFSET(ZToggleText, m_vecTextPtr, 0xa8);
    RE_VERIFY_OFFSET(ZToggleText, m_vecResourceTextPtr, 0xb4);
}
