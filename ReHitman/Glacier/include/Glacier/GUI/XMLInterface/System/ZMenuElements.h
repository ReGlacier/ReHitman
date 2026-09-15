#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZXMLGUISystem;
    class IOptionsInterface;
    class IMenuFactoryInterface;
    class ZGUIBase;
    class IGUIElement;
    class IGUIElement;
    class IWindowInterface;
    class ZColorSet;
    class ZButtonGraphic;
    class ZListDataInterface;
    class ITableDataInterface;


    class ZMenuElements
    {
    public:
        // types
        enum EGUIBaseType
        {
            EGUI_ELEMENT = 0,
            EWINDOW = 1,
            ECOLOR_SET = 2,
            EBUTTON_GRAPHIC = 3,
            EMISC = 4,
        };

        struct SGUIBaseType
        {
            ZGUIBase* pGUIBase;
            EGUIBaseType eType;
        };

        // constants
        static constexpr int NUM_OF_GUIBASETYPE = 1024;
        static constexpr int NUM_OF_MENU_FACTORIES = 4;

        // vtbl
        virtual ~ZMenuElements();
        virtual void Init();
        virtual void ReadXML() = 0;
        virtual void WindowClosed(const char* pszWindowName);
        virtual IOptionsInterface* GetOptionsInterface();
        virtual int GetActivatedBy(int lWhat);

        // methods
        ZMenuElements();

        ZGUIBase* Create(const char* pszTag, const char* pszType);
        IGUIElement* GetGUIElement(const char* pszName);
        IGUIElement* GetGUIElement(int iIndex);
        IWindowInterface* GetWindow(const char* pszName);
        ZColorSet* GetColorSet(const char* pszName);
        ZButtonGraphic* GetButtonGraphic(const char* pszName);
        ZListDataInterface* GetListDataInterface(const char* pszName);
        ITableDataInterface* GetTableDataInterface(const char* pszName);

        void AddFactory(IMenuFactoryInterface* pMenuFactory);
        void ReloadMenuElements();
        void ReadFiles(const char** apszFiles);
        void DeleteMenuElements();
        IGUIElement* CreateGUIElement(const char* pszType);
        IWindowInterface* CreateGUIWindow(const char* pszType);
        ZColorSet* CreateColorSet();
        ZButtonGraphic* CreateButtonGraphic();
        ZGUIBase* CreateMisc(const char* pszTag);
        ZGUIBase* GetGUIBase(const char*, EGUIBaseType eType);
        int AddGUIBase(ZGUIBase*, EGUIBaseType eType);

        // members
        ZXMLGUISystem* m_pXMLGUISystem; // +0x04 - Verified by Ctor2
        SGUIBaseType m_aGUIBaseType[NUM_OF_GUIBASETYPE]; // +0x08 - Verified by GetGUIElement(int)
        int32_t m_iNumOfGUIBaseType; // +0x2008 - Verified by GetGUIElement(int)
        IMenuFactoryInterface* m_apMenuFactory[NUM_OF_MENU_FACTORIES]; // +0x200C - Verified by Ctor2
        int32_t m_iNumOfMenuFactories; // +0x201C - Verified by Ctor2
        IMenuFactoryInterface* m_pStandardFactory; // +0x2020 - Verified by Ctor2
    };
    RE_VERIFY_SIZE(ZMenuElements, 0x2024); // Verified by PC Ctor2
    RE_VERIFY_OFFSET(ZMenuElements, m_pXMLGUISystem, 0x04);
    RE_VERIFY_OFFSET(ZMenuElements, m_aGUIBaseType, 0x08);
    RE_VERIFY_OFFSET(ZMenuElements, m_iNumOfGUIBaseType, 0x2008);
    RE_VERIFY_OFFSET(ZMenuElements, m_apMenuFactory, 0x200C);
    RE_VERIFY_OFFSET(ZMenuElements, m_iNumOfMenuFactories, 0x201C);
    RE_VERIFY_OFFSET(ZMenuElements, m_pStandardFactory, 0x2020);
}
