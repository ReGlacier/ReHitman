#pragma once

#include <Glacier/GUI/XMLInterface/System/IMenuFactoryInterface.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIDataInterfaces.h>


namespace Glacier
{
    class ZGUIBase;
    class ZStandardWindow;
    class ZMenuElements;

    class ZStandardMenuFactory : public IMenuFactoryInterface
    {
    public:
        // vtbl
        ZListDataInterface* GetListDataInterface(const char* pszName) override;
        ITableDataInterface* GetTableDataInterface(const char* pszName) override;
        IWindowInterface* WindowFactory(const char* pszName, ZMenuElements* pMenuElements) override;
        IGUIElement* GUIElementFactory(const char* pszName) override;
    };
}
