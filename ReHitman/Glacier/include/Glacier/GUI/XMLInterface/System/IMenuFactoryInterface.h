#pragma once


namespace Glacier
{
    // fwds
    class ZMenuElements;
    class IWindowInterface;
    class IGUIElement;
    class ZListDataInterface;
    class ITableDataInterface;

    class IMenuFactoryInterface
    {
    public:
        // vtbl
        virtual ZListDataInterface* GetListDataInterface(const char* pszName) = 0;
        virtual ITableDataInterface* GetTableDataInterface(const char* pszName) = 0;
        virtual IWindowInterface* WindowFactory(const char* pszName, ZMenuElements* pMenuElements) = 0;
        virtual IGUIElement* GUIElementFactory(const char* pszName) = 0;
    };
    RE_VERIFY_SIZE(IMenuFactoryInterface, 0x4);
}
