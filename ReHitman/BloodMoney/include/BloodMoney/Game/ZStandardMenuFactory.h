#pragma once

namespace Hitman::BloodMoney
{
    class ZGUIBase;
    class ZStandardWindow;
    class ZHM3MenuElements;

    class ZStandardMenuFactory
    {
    public:
        virtual void* GetListDataInteface(const char* dataListTypeName);
        virtual void* GetTableDataInterface(const char*); // DO NOT USE THIS! NOT IMPLEMNETED IN BLOOD MONEY
        virtual ZStandardWindow* WindowFactory(const char* windowName, ZHM3MenuElements* menuElements);
        virtual ZGUIBase* GUIElementFactory(const char* elementName);
    };
}