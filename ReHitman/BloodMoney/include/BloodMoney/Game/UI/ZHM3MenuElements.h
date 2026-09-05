#pragma once

namespace Glacier
{
    class ZGUIBase;
    class ZXMLGUISystem;
    class ZStandardMenuFactory;
}

namespace Hitman::BloodMoney
{
    class ZHM3MenuFactory;
    class ZOnlineElements;

    class ZHM3MenuElements
    {
    public:
        /// === members ===
        Glacier::ZXMLGUISystem*			m_XMLGUISystem; //0x0004
        char					pad_0008[8196]; //0x0008
        Glacier::ZStandardMenuFactory*	m_standardMenuFactory_1; //0x200C
        ZHM3MenuFactory*		m_hm3MenuFactory_1; //0x2010
        ZOnlineElements*		m_onlineElementsFactory; //0x2014
        char					pad_2018[8]; //0x2018
        Glacier::ZStandardMenuFactory*	m_standardMenuFactory; //0x2020
        ZHM3MenuFactory*		m_hm3MenuFactory; //0x2024

        /// === vftable ===
        virtual void Release(bool);
        virtual void Init();
        virtual void ReadXML();
        virtual void WindowClosed(const char* menuName);
        virtual void GetOptionsInterface();
        virtual void GetActivatedBy(int);

        // === api ===
        Glacier::ZGUIBase* GetGUIElement(const char* psElementName);
    };
}
