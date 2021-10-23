#pragma once

#include <BloodMoney/Game/UI/ZGUIBase.h>
#include <BloodMoney/Game/UI/IGUIElement.h>

namespace Hitman::BloodMoney {
    class ZFRAME;

    enum ENavigation {
        ENAV_HORIZONTAL,
        ENAV_VERTICAL
    };

    enum EGUIAlignType {
        //Horizontal
        AT_H_Left = 1,
        AT_H_Right = 2,
        AT_H_Center = 4,
        //Vertical
        AT_V_Top = 16,
        AT_V_Bottom = 32,
        AT_V_Center = 64
    };

    class IWindowInterface : public ZGUIBase {
    public:
        //vftable
        virtual void OpenWindow(ZResourceManager* pResourceManager, bool, ZWINGROUP* pWinGroup, bool);  //pure virtual
        virtual void CloseWindow(ZResourceManager* pResourceManager, bool); //pure virtual
        virtual void PushSubWindow(const char* psName, IGUIElement* pGuiElement, bool, bool);
        virtual void PopSubWindow();
        virtual void Update(int); //pure virtual
        virtual ZWINDOW* GetTopSubWindow();
        virtual ZWINDOW* GetButtomSubWindow();
        virtual IGUIElement* GetCurrentElements();
        virtual void Cancel();
        virtual void Invalidate();
        virtual void GrapFocus();
        virtual bool IsDialog();
        virtual bool IsTRC();
        virtual int GetTRCPriority();
        virtual void Click(IGUIElement* pGuiElement, int /*eZWUserEvents*/, unsigned int);

        /**
         * Size reconstruction:
         * Here we have 2 inherited things:
         * 1) ZStandardWindow  - size is 0x15C
         * 2) ZComponentWindow - size is 0x39C
         *
         * And there are contains first field at 0x90. So, I guess that total size of base class is 0x90
         */
        //data (total size is 0x90, base size is 0x4C)
        int m_field4C;
        bool m_bPopOnBack;
        bool field_51;
        bool field_52;
        bool field_53;
        const char* m_psBackground;
        int m_field58;
        int m_field5C;
        const char* m_psRollBackWindow;
        int m_field64;
        int m_field68;
        ZWINGROUP* m_pGraphicGroup;
        ZFRAME* m_pFrame;
        int m_iWidth;
        int m_iHeight;
        bool m_bUseBackgroundImage;
        bool m_bFrame;
        bool field_7E;
        bool field_7F;
        int m_iAlignment; //it's mask, see EGUIAlignType for details
        int m_iLineSpace;
        ENavigation m_eNavigation;
        bool m_bIsOpened;
        bool m_bRollBackMark;
        bool m_bPauseEngine;
        bool field_8F;
    };
}