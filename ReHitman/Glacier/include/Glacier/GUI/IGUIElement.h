#pragma once

#include <Glacier/GUI/ZGUIBase.h>
#include <Glacier/GUI/eZWUserEvents.h>
#include <Glacier/GUI/ZColorSet.h>


namespace Glacier
{
    // fwds
    class ZWINDOW;
    class ZXMLGUISystem;
    class ZResourceManager;
    class ZButtonGraphic;

    enum EFontType
    {
        FT_HEADER = 0,
        FT_MENU = 1,
        FT_TEXT = 2,
        NUM_OF_FONTS = 3,
    };

    enum EAlignment
    {
        ERIGHT = 0,
        ELEFT = 1,
        ECENTER = 2,
    };

    class IGUIElement : public ZGUIBase
    {
    public:
        // vtbl
        virtual void Update(bool);
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*);
        virtual void OpenSubwindow();
        virtual void SetValue(int);
        virtual void Use();
        virtual void FocusUpdate();
        virtual void Setup(float*, ZResourceManager* pResourceManager, ZWINGROUP* pGroup);
        virtual void ReleaseResources(ZResourceManager* pResourceManager);
        virtual bool SetFocus(bool);
        virtual void Cancel();
        virtual void Invalidate();
        virtual ZWINDOW* GetWindow();

        // methods
        void ChangeColorSet(ZWINGROUP* pGroup, ZColorSet* pColorSet);
        void ChangeColor(ZWINGROUP* pGroup, ZColorSet* pColorSet, ZColorSet::EColorIndex eColorIndex);
        void SetColor(uint32_t rgba, ZWINGROUP* pGroup, uint32_t mask = 0xFFFFFFFFu);
        void GetRightPosOfTextGroup(ZWINGROUP*, Glacier::Vector3* pPos, int iOffsetByX);

        // members
        ZColorSet* m_pColorSet;
        int32_t m_iIndex;
        int32_t m_iControlPrompt;
        EFontType m_eFontType;
        EAlignment m_eAlignment;
        ZButtonGraphic* m_pButtonGraphic;
        bool m_bShadow;
    };
    RE_VERIFY_SIZE(IGUIElement, 0x68); // Verified by PS2 and previous comment
}
