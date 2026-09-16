#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggle.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZToggleTextItem;

    class ZToggleTextSimple : public ZToggle
    {
    public:
        // constants
        static constexpr int32_t NUM_OF_TEXT_STRINGS = 8;

        // vtbl
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual void OptionChanged() override;

        // methods
        ZToggleTextSimple();
        ~ZToggleTextSimple();

        // members
        zstring m_asTextValues[NUM_OF_TEXT_STRINGS]; // +0xa4
        int32_t m_iNumOfTextAdded;                   // +0x104
    };
    RE_VERIFY_SIZE(ZToggleTextSimple, 0x108);
    RE_VERIFY_OFFSET(ZToggleTextSimple, m_asTextValues, 0xa4);
    RE_VERIFY_OFFSET(ZToggleTextSimple, m_iNumOfTextAdded, 0x104);
}
