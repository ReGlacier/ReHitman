#pragma once

#include <Glacier/GUI/XMLInterface/System/IMenuFactoryInterface.h>


namespace Glacier
{
    class ZGUIBase;
    class ZStandardWindow;
    class ZMenuElements;

    class ZListDataInterface
    {
    public:
        // vtbl
        // TODO: Finish me
    };

    class ITableDataInterface
    {
    public:
        // vtbl
        // TODO: Finish me
    };

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
