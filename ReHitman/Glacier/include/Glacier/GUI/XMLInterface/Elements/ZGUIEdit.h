#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZBUTTON;
    class ZLINEOBJ;

    class ZGUIEdit : public IGUIElement
    {
    public:
        // vtbl
        virtual void Done();
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;
        virtual void Update(bool) override;
        virtual bool SetFocus(bool) override;

        // methods
        ZGUIEdit();
        ~ZGUIEdit();

        void SetText(zstring strText);
        zstring GetText() const;

        // members
        float m_fCursorTime;           // +0x68
        bool m_bCursor;                // +0x6c
        ZWINGROUP* m_pWinGroup_Cursor; // +0x70
        ZBUTTON* m_pButton_Text;       // +0x74
        char m_sText[256];             // +0x78
        uint32_t m_dwLen;              // +0x178
        char m_szCursor[128];          // +0x17c
        bool m_bStateActive;           // +0x1fc
        int32_t m_dwTextLimit;         // +0x200
        int32_t m_iTextPixelLimit;     // +0x204
        bool m_bPassword;              // +0x208
        zstring m_szInfoText;          // +0x20c
        ZStaticVector<ZLINEOBJ*, 8> m_LineObjs; // +0x218
        bool m_bDisableAnimateAlpha;   // +0x23c
        bool m_bUseCharList;           // +0x23d

    private:
        void UpdateText();
    };
    RE_VERIFY_SIZE(ZGUIEdit, 0x240);
    RE_VERIFY_OFFSET(ZGUIEdit, m_fCursorTime, 0x68);
    RE_VERIFY_OFFSET(ZGUIEdit, m_bCursor, 0x6c);
    RE_VERIFY_OFFSET(ZGUIEdit, m_pWinGroup_Cursor, 0x70);
    RE_VERIFY_OFFSET(ZGUIEdit, m_pButton_Text, 0x74);
    RE_VERIFY_OFFSET(ZGUIEdit, m_sText, 0x78);
    RE_VERIFY_OFFSET(ZGUIEdit, m_dwLen, 0x178);
    RE_VERIFY_OFFSET(ZGUIEdit, m_szCursor, 0x17c);
    RE_VERIFY_OFFSET(ZGUIEdit, m_bStateActive, 0x1fc);
    RE_VERIFY_OFFSET(ZGUIEdit, m_dwTextLimit, 0x200);
    RE_VERIFY_OFFSET(ZGUIEdit, m_iTextPixelLimit, 0x204);
    RE_VERIFY_OFFSET(ZGUIEdit, m_bPassword, 0x208);
    RE_VERIFY_OFFSET(ZGUIEdit, m_szInfoText, 0x20c);
    RE_VERIFY_OFFSET(ZGUIEdit, m_LineObjs, 0x218);
    RE_VERIFY_OFFSET(ZGUIEdit, m_bDisableAnimateAlpha, 0x23c);
    RE_VERIFY_OFFSET(ZGUIEdit, m_bUseCharList, 0x23d);
}
