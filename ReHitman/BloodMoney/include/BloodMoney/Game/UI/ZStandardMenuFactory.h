#pragma once

namespace Hitman::BloodMoney
{
    class ZGUIBase;
    class ZStandardWindow;
    class ZHM3MenuElements;

    struct ZListDataInterface
    {};

    struct ITableDataInterface
    {};

    class ZStandardMenuFactory
    {
    public:
        virtual ZListDataInterface* GetListDataInteface(const char* dataListTypeName);
        virtual ITableDataInterface* GetTableDataInterface(const char*);
        virtual ZStandardWindow* WindowFactory(const char* windowName, ZHM3MenuElements* menuElements);
        virtual ZGUIBase* GUIElementFactory(const char* elementName);
    };
}