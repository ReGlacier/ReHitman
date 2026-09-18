#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>


namespace Glacier
{
    // fwds
    class ZBUTTON;
    class ZWINGROUP;
    class ITableDataInterface;

    class ZGUITable : public IGUIElement
    {
    public:
        static constexpr int32_t MAX_NUM_OF_COLUMNS = 5;
        static constexpr int32_t MAX_NUM_OF_CELLS = 25;

        // vtbl
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual bool SetFocus(bool) override;

        // methods
        ZGUITable();

        // members
        ITableDataInterface* m_pTableDataInterface; // +0x68
        ZBUTTON* m_paButtons[7];                    // +0x6c
        ZWINGROUP* m_paCells[MAX_NUM_OF_CELLS];     // +0x88
        int32_t m_iNumOfRows;                       // +0xec
        int32_t m_iNumOfColumns;                    // +0xf0
        int32_t m_iNumOfRowsDisplayed;              // +0xf4
        int32_t m_iTopRow;                          // +0xf8
        int32_t m_iColumnSize[MAX_NUM_OF_COLUMNS];  // +0xfc
        int32_t m_iLineDistance;                    // +0x110

    private:
        virtual void SetTextOnTable();
    };
    RE_VERIFY_SIZE(ZGUITable, 0x114);
    RE_VERIFY_OFFSET(ZGUITable, m_pTableDataInterface, 0x68);
    RE_VERIFY_OFFSET(ZGUITable, m_paButtons, 0x6c);
    RE_VERIFY_OFFSET(ZGUITable, m_paCells, 0x88);
    RE_VERIFY_OFFSET(ZGUITable, m_iNumOfRows, 0xec);
    RE_VERIFY_OFFSET(ZGUITable, m_iNumOfColumns, 0xf0);
    RE_VERIFY_OFFSET(ZGUITable, m_iNumOfRowsDisplayed, 0xf4);
    RE_VERIFY_OFFSET(ZGUITable, m_iTopRow, 0xf8);
    RE_VERIFY_OFFSET(ZGUITable, m_iColumnSize, 0xfc);
    RE_VERIFY_OFFSET(ZGUITable, m_iLineDistance, 0x110);
}
