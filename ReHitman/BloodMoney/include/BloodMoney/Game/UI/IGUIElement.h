#pragma once

#include <BloodMoney/Game/UI/ZGUIBase.h>
#include <BloodMoney/Game/UI/eZWUserEvents.h>
#include <BloodMoney/Game/UI/ZColorSet.h>

namespace Hitman::BloodMoney {
    class ZWINDOW;
    class ZXMLGUISystem;
    class ZResourceManager;

    class IGUIElement : public ZGUIBase {
    public:
        //pub
        enum class EAlignment {
            Left = 1,
            Right = 0,
            Center = 2
        };

        enum class EFontType {
            Header = 0,
            Menu = 1,
            Text = 2
        };

        //vftable
        virtual void Update(bool);
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*);
        virtual void OpenSubwindow();
        virtual void SetValue(int);
        virtual void Use();
        virtual void FocusUpdate();
        virtual void Setup(float*, ZResourceManager* pResourceManager, ZWINGROUP* pGroup);
        virtual void ReleaseResources(ZResourceManager* pResourceManager);
        virtual void SetFocus(bool);
        virtual void Cancel();
        virtual void Invalidate();
        virtual ZWINDOW* GetWindow();

        //api
        void ChangeColorSet(ZWINGROUP* pGroup, ZColorSet* pColorSet);
        void ChangeColor(ZWINGROUP* pGroup, ZColorSet* pColorSet, ZColorSet::EColorIndex eColorIndex);
        void SetColor(uint32_t rgba, ZWINGROUP* pGroup, uint32_t mask = 0xFFFFFFFFu);
        void GetRightPosOfTextGroup(ZWINGROUP*, Glacier::Vector3* pPos, int iOffsetByX);

        //data (total size is 0x6C, base size is 0x48)
        ZColorSet* m_pColorSet;
        int m_field4C;
        int m_field50;
        EFontType m_eFontType;
        EAlignment m_eAlignment;
        int m_pButtonGraphic;
        bool m_bShadow;
        bool m_field61;
        bool m_field62;
        bool m_field63;
        int m_field64;
        int m_iActivateButtonKeyCode;

        /*
         * @note:
         *  Why the size of this class is 0x6C?
         *  Because there are no direct allocations of IGUIElement instance without any inheritance
         *  we need to find a class with the lowest size and small inheritance tree and if it exists we can choose
         *  the size of that class as the base size (means that this class does not contain any additional information).
         *  There are presented the class list which selected to find base size:
         *
         * Class                   Size (HEX, in bytes)
         * -----------------------------
         * ZBackgroundCameraClear  0x6C
         * ZHM3HintDetail          0x6C
         * ZAnimation              0x80
         * ZTextBlock              0x80
         * ZXMLFrame               0x80
         * ZGraphic                0x84
         * ZWeaponDetails          0x110
         * ZUpgradeInfo            0x110
         * ZGUIEdit                0x240
         * ZWeaponSelector         0x564
         * ZListBox                0xC14
         * ZGUIList                0x1A48
         */
    };
}