#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>


namespace Glacier
{
    class ZComponentWindow : public IWindowInterface
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual void OpenWindow(ZResourceManager* pResourceManager, bool, ZWINGROUP* pWinGroup, bool) override;
        virtual void CloseWindow(ZResourceManager* pResourceManager, bool) override;
        virtual void PushSubWindow(const char* psName, IGUIElement* pGuiElement, bool, bool) override;
        virtual bool PopSubWindow() override;
        virtual bool Update(int) override;
        virtual ZWINDOW* GetTopSubWindow() override;
        virtual ZWINDOW* GetButtomSubWindow() override;
        virtual void Cancel() override;
        virtual void Invalidate() override;
        virtual void GrapFocus() override;

        // methods
        ZComponentWindow(ZMenuElements* pMenuElements);

        // members
        ZStaticVector<IWindowInterface*, 8> m_WindowStack;
        ZStaticVector<IGUIElement*, 44> m_aGUIElements[2];
        ZStaticVector<IGUIElement*, 44> m_aGUICurrentElements[2];
        ZVector2 m_v2BrowserOffset;
        ZResourceManager* m_pResourceManager;
        ZWINGROUP* m_pSubwindowParent;
        bool m_bSubWindowFocus;
        ZWINGROUP* m_pParent;

    private:
        void OpenSubWindow(IWindowInterface* pWindow, bool bReset, ZWINGROUP* pWinGroup, bool bFocus);
    };
    RE_VERIFY_SIZE(ZComponentWindow, 0x39C);
}
