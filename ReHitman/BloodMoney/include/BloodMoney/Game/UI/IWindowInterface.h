#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/zstring.h>
#include <BloodMoney/Game/UI/ZGUIBase.h>
#include <BloodMoney/Game/UI/IGUIElement.h>

namespace Hitman::BloodMoney 
{
    class ZFRAME;
    class ZResourceManager;

    enum ENavigation 
    {
        EVERTICAL = 0,
        EHORIZONTAL = 1,
    };


    class IWindowInterface : public ZGUIBase 
    {
    public:
        // types
        struct ZElementExtraInfo 
        {
            IGUIElement* m_pGUIElement;
            int32_t      m_iAlignmentOverride;
        };

        //vftable
        virtual void OpenWindow(ZResourceManager* pResourceManager, bool, ZWINGROUP* pWinGroup, bool);
        virtual void CloseWindow(ZResourceManager* pResourceManager, bool);
        virtual void PushSubWindow(const char* psName, IGUIElement* pGuiElement, bool, bool);
        virtual void PopSubWindow();
        virtual void Update(int);
        virtual ZWINDOW* GetTopSubWindow();
        virtual ZWINDOW* GetButtomSubWindow();
        virtual IGUIElement* GetCurrentElements();
        virtual void Cancel();
        virtual void Invalidate();
        virtual void GrapFocus();
        virtual bool IsDialog();
        virtual bool IsTRC();
        virtual int GetTRCPriority();
        virtual void Click(IGUIElement* pGuiElement, eZWUserEvents, unsigned int);

        //data (total size is 0x90, base size is 0x4C)
        ZResourceManager* m_pResourceManager;
        bool m_bPopOnBack;
        Glacier::zstring m_sBackground;
        Glacier::zstring m_sRollBackWindow;
        ZWINGROUP* m_pWinGroupBackGround;
        ZFRAME* m_pFrameBackground;
        Glacier::ZVector2 m_v2WindowSize;
        bool m_bUseBackgroundIngame;
        bool m_bBackgroundIsFrame;
        uint32_t m_iWindowAlignment;
        int32_t m_iLineSpace;
        ENavigation m_eNavigation;
        bool m_bOpen;
        bool m_bRollBackMark;
        bool m_bPauseEngine;
    };
    RE_VERIFY_SIZE(IWindowInterface, 0x90); // Verified
}