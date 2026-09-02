#pragma once


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

    class ZStandardMenuFactory
    {
    public:
        // vtbl
        virtual ZListDataInterface* GetListDataInteface(const char* dataListTypeName);
        virtual ITableDataInterface* GetTableDataInterface(const char*);
        virtual ZStandardWindow* WindowFactory(const char* windowName, ZMenuElements* menuElements);
        virtual ZGUIBase* GUIElementFactory(const char* elementName);
    };
}
