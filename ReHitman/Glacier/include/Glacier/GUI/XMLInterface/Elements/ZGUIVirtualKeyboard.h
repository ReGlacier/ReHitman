#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/UTC4.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZWINGROUP;

    class ZGUIVirtualKeyboard : public IGUIElement
    {
    public:
        // types
        enum EKEYBOARD
        {
            KEYBOARD_NORMAL = 0,
            KEYBOARD_PASSWORD = 1,
        };

        enum EMODE
        {
            MODE_ALPHABET = 0,
            MODE_CONTROL = 1,
        };

        // vtbl
        virtual ~ZGUIVirtualKeyboard();
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void Update(bool) override;
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual void Done(ZXMLGUISystem*);

        // methods
        ZGUIVirtualKeyboard();

        void SetTextUC(const UTC4CHAR* pText);
        void GetTextUC(UTC4CHAR* pText) const;
        void SetText(zstring strText);
        zstring GetText() const;

        // members
        ZWINGROUP* m_pParent;                          // +0x68
        ZWINGROUP* m_pWinGroup_BackgroundPassword;     // +0x6c
        ZWINGROUP* m_pWinGroup_BackgroundNormal;       // +0x70
        ZWINGROUP* m_pWinGroup_BackgroundCaps;         // +0x74
        ZWINGROUP* m_pWinGroup_ButtonSmall;            // +0x78
        ZWINGROUP* m_pWinGroup_ButtonBig;              // +0x7c
        ZWINGROUP* m_pWinGroup_ButtonBigHighLightCaps; // +0x80
        ZWINGROUP* m_pWinGroup_ButtonBigHighLight;     // +0x84
        ZWINGROUP* m_pWinGroup_ButtonSmallHighLight;   // +0x88
        ZWINGROUP* m_pWinGroup_Cursor;                 // +0x8c
        ZWINGROUP* m_pWinGroup_Text;                   // +0x90
        ZWINGROUP* m_pWinGroup_Title;                  // +0x94
        uint32_t m_dwPosSmallX;                        // +0x98
        uint32_t m_dwPosSmallY;                        // +0x9c
        uint32_t m_dwPosBigX;                          // +0xa0
        uint32_t m_dwPosBigY;                          // +0xa4
        EKEYBOARD m_eKeyboard;                         // +0xa8
        EMODE m_eMode;                                 // +0xac
        bool m_bCaps;                                  // +0xb0
        float m_fSmallButtonHighLight;                 // +0xb4
        float m_fBigButtonHighLight;                   // +0xb8
        UTC4CHAR m_utc4Text[256];                      // +0xbc
        uint32_t m_dwTextLen;                          // +0x4bc
        bool m_bCursor;                                // +0x4c0
        float m_fCursorTime;                           // +0x4c4
        zstring m_sTitle;                              // +0x4c8
        int32_t m_dwTextLimit;                         // +0x4d4
        int32_t m_iTextPixelLimit;                     // +0x4d8

    private:
        void UpdateText();
        void AddChar(uint32_t uChar);
        void DelChar();
    };
    RE_VERIFY_SIZE(ZGUIVirtualKeyboard, 0x4DC);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pParent, 0x68);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_BackgroundPassword, 0x6c);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_BackgroundNormal, 0x70);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_BackgroundCaps, 0x74);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_ButtonSmall, 0x78);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_ButtonBig, 0x7c);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_ButtonBigHighLightCaps, 0x80);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_ButtonBigHighLight, 0x84);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_ButtonSmallHighLight, 0x88);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_Cursor, 0x8c);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_Text, 0x90);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_pWinGroup_Title, 0x94);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_dwPosSmallX, 0x98);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_dwPosSmallY, 0x9c);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_dwPosBigX, 0xa0);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_dwPosBigY, 0xa4);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_eKeyboard, 0xa8);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_eMode, 0xac);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_bCaps, 0xb0);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_fSmallButtonHighLight, 0xb4);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_fBigButtonHighLight, 0xb8);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_utc4Text, 0xbc);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_dwTextLen, 0x4bc);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_bCursor, 0x4c0);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_fCursorTime, 0x4c4);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_sTitle, 0x4c8);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_dwTextLimit, 0x4d4);
    RE_VERIFY_OFFSET(ZGUIVirtualKeyboard, m_iTextPixelLimit, 0x4d8);
}
