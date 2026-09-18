#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/zvector.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>


namespace Glacier
{
    class ZStandardWindow : public IWindowInterface
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual void OpenWindow(ZResourceManager* pResourceManager, bool, ZWINGROUP* pWinGroup, bool) override;
        virtual void CloseWindow(ZResourceManager* pResourceManager, bool) override;
        virtual bool Update(int) override;
        virtual const ZStaticVector<IGUIElement*, 44>* GetCurrentElements() override;
        virtual void Cancel() override;
        virtual void Invalidate() override;
        virtual void GrapFocus() override;
        virtual bool IsDialog() override;
        virtual bool IsTRC() override;
        virtual int GetTRCPriority() override;

        // methods
        ZStandardWindow(ZMenuElements* pMenuElements);

        // members
        zvector<IWindowInterface::ZElementExtraInfo> m_vecGUIElementExtraInfo;
        ZStaticVector<IGUIElement*, 44> m_vecCurrentElements;
        int32_t m_iAlignmentId;
        bool m_bDialogWindow;
        bool m_bTRCWindow;
        int m_iTRCPriority;
    };
    RE_VERIFY_SIZE(ZStandardWindow, 0x15C); // Verified
}
