#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/Windows/ZGUIElementLink.h>


namespace Glacier
{
    // fwds
    class ZFRAME;
    class ZResourceManager;
    class ZCONTROL;


    class IWindowInterface : public ZGUIBase
    {
    public:
        // types
        struct ZElementExtraInfo
        {
            IGUIElement* m_pGUIElement;
            int32_t      m_iAlignmentOverride;

            ZElementExtraInfo();
            ZElementExtraInfo(IGUIElement* pGUIElement, ENavigation eNavigation, int32_t iAlignmentId);
            ZElementExtraInfo& operator=(const ZElementExtraInfo& other);
            ENavigation GetNavigation();
            int32_t GetNavigationId();
        };

        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void OpenWindow(ZResourceManager* pResourceManager, bool, ZWINGROUP* pWinGroup, bool);
        virtual void CloseWindow(ZResourceManager* pResourceManager, bool);
        virtual void PushSubWindow(const char* psName, IGUIElement* pGuiElement, bool, bool);
        virtual bool PopSubWindow();
        virtual bool Update(int);
        virtual ZWINDOW* GetTopSubWindow();
        virtual ZWINDOW* GetButtomSubWindow();
        virtual const ZStaticVector<IGUIElement*, 44>* GetCurrentElements();
        virtual void Cancel();
        virtual void Invalidate();
        virtual void GrapFocus();
        virtual bool IsDialog();
        virtual bool IsTRC();
        virtual int GetTRCPriority();
        virtual void Click(IGUIElement* pGuiElement, eZWUserEvents, uint32_t);

        // methods
        IWindowInterface(ZMenuElements* pMenuElements);
        void GetTopLeftPos(ZVector2& rResult, ZVector2& rViewport);

        // members
        ZResourceManager* m_pResourceManager;
        bool m_bPopOnBack;
        zstring m_sBackground;
        zstring m_sRollBackWindow;
        ZWINGROUP* m_pWinGroupBackGround;
        ZFRAME* m_pFrameBackground;
        ZVector2 m_v2WindowSize;
        bool m_bUseBackgroundIngame;
        bool m_bBackgroundIsFrame;
        uint32_t m_iWindowAlignment;
        int32_t m_iLineSpace;
        ENavigation m_eNavigation;
        bool m_bOpen;
        bool m_bRollBackMark;
        bool m_bPauseEngine;

    protected:
        ZGUIElementLink SetupGUIElements(float* pfPos, ENavigation eNavigation, ZStaticVector<IGUIElement*, 44>& rElements,
            ZResourceManager* pResourceManager, ZWINGROUP* pParent, ZGUIElementLink* pPrevElementLink);
        ZCONTROL* GetUpLink(int iIndex, const ZStaticVector<ZGUIElementLink, 44>& rElements);
        ZCONTROL* GetDownLink(int iIndex, const ZStaticVector<ZGUIElementLink, 44>& rElements);
        ZCONTROL* GetLeftLink(int iIndex, const ZStaticVector<ZGUIElementLink, 44>& rElements);
        ZCONTROL* GetRightLink(int iIndex, const ZStaticVector<ZGUIElementLink, 44>& rElements);
        void SetFirstFocus(ZStaticVector<IGUIElement*, 44>& rElements);
        void CallCancel(ZStaticVector<IGUIElement*, 44>& rElements);
    };
    RE_VERIFY_SIZE(IWindowInterface, 0x90); // Verified
}
