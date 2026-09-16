#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphicPart.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphicPartType.h>
#include <Glacier/GUI/XMLInterface/Elements/ZColorSet.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUIGroup.h>
#include <Glacier/GUI/XMLInterface/Elements/ZToggleTextItem.h>
#include <Glacier/GUI/XMLInterface/Readers/ZGUIXMLReader2.h>
#include <Glacier/GUI/XMLInterface/System/IMenuFactoryInterface.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIDataInterfaces.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/System/ZStandardMenuFactory.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/ZSTL/ZFilePath.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    ZMenuElements::ZMenuElements()
    {
        m_pXMLGUISystem = nullptr;
        m_iNumOfGUIBaseType = 0;
        m_iNumOfMenuFactories = 0;

        m_pStandardFactory = static_cast<IMenuFactoryInterface*>(ZUniMemory::Allocate(sizeof(ZStandardMenuFactory)));
        if (m_pStandardFactory)
            new (m_pStandardFactory) ZStandardMenuFactory();

        ZASSERT(m_iNumOfMenuFactories >= 1);
        m_apMenuFactory[m_iNumOfMenuFactories++] = m_pStandardFactory;
    }

    ZMenuElements::~ZMenuElements()
    {
        DeleteMenuElements();
        ZUniMemory::Free(m_pStandardFactory);
    }

    void ZMenuElements::Init()
    {
    }

    void ZMenuElements::WindowClosed(const char* pszWindowName)
    {
    }

    IOptionsInterface* ZMenuElements::GetOptionsInterface()
    {
        return nullptr;
    }

    int ZMenuElements::GetActivatedBy(int lWhat)
    {
        return 0;
    }

    ZGUIBase* ZMenuElements::Create(const char* pszTag, const char* pszType)
    {
        if (pszType && strcmp(pszTag, "GuiElement") == 0)
            return static_cast<ZGUIBase*>(CreateGUIElement(pszType));

        if (strcmp(pszTag, "ColorSetDefinition") == 0)
            return static_cast<ZGUIBase*>(CreateColorSet());

        if (strcmp(pszTag, "ButtonGraphicDefinition") == 0)
            return static_cast<ZGUIBase*>(CreateButtonGraphic());

        if (strcmp(pszTag, "Window") != 0)
            return CreateMisc(pszTag);

        return static_cast<ZGUIBase*>(CreateGUIWindow(pszType ? pszType : "Standard"));
    }

    IGUIElement* ZMenuElements::GetGUIElement(const char* pszName)
    {
        return static_cast<IGUIElement*>(GetGUIBase(pszName, EGUI_ELEMENT));
    }

    IGUIElement* ZMenuElements::GetGUIElement(int iIndex)
    {
        if (iIndex >= m_iNumOfGUIBaseType)
            return nullptr;

        ZASSERT(m_aGUIBaseType[iIndex].eType == EGUI_ELEMENT);
        return static_cast<IGUIElement*>(m_aGUIBaseType[iIndex].pGUIBase);
    }

    IWindowInterface* ZMenuElements::GetWindow(const char* pszName)
    {
        ZGUIBase* pBase = GetGUIBase(pszName, EWINDOW);
        return pBase ? static_cast<IWindowInterface*>(pBase) : nullptr;
    }

    ZColorSet* ZMenuElements::GetColorSet(const char* pszName)
    {
        return static_cast<ZColorSet*>(GetGUIBase(pszName, ECOLOR_SET));
    }

    ZButtonGraphic* ZMenuElements::GetButtonGraphic(const char* pszName)
    {
        return static_cast<ZButtonGraphic*>(GetGUIBase(pszName, EBUTTON_GRAPHIC));
    }

    ZListDataInterface* ZMenuElements::GetListDataInterface(const char* pszName)
    {
        for (int32_t i = m_iNumOfMenuFactories - 1; i >= 0; --i)
        {
            ZListDataInterface* pInterface = m_apMenuFactory[i]->GetListDataInterface(pszName);
            if (pInterface)
                return pInterface;
        }

        return nullptr;
    }

    ITableDataInterface* ZMenuElements::GetTableDataInterface(const char* pszName)
    {
        for (int32_t i = m_iNumOfMenuFactories - 1; i >= 0; --i)
        {
            ITableDataInterface* pInterface = m_apMenuFactory[i]->GetTableDataInterface(pszName);
            if (pInterface)
                return pInterface;
        }

        return nullptr;
    }

    void ZMenuElements::AddFactory(IMenuFactoryInterface* pMenuFactory)
    {
        ZASSERT(m_iNumOfMenuFactories < NUM_OF_MENU_FACTORIES);
        m_apMenuFactory[m_iNumOfMenuFactories++] = pMenuFactory;
    }

    void ZMenuElements::ReloadMenuElements()
    {
        DeleteMenuElements();
        ReadXML();
    }

    void ZMenuElements::ReadFiles(const char** apszFiles)
    {
        ZGUIXMLReader2* pReader = static_cast<ZGUIXMLReader2*>(ZUniMemory::Allocate(sizeof(ZGUIXMLReader2)));
        if (pReader)
            new (pReader) ZGUIXMLReader2();

        if (pReader && *apszFiles)
        {
            const char** pCurrentFile = apszFiles;
            do
            {
                ZFilePath filePath;
                filePath.Clear();
                ZFilePath menuPath("Menu");
                filePath.Append(menuPath);
                ZFilePath fileName(*pCurrentFile);
                filePath.Append(fileName);
                pReader->XMLParse(filePath.AsChar(), this);
                ++pCurrentFile;
            }
            while (*pCurrentFile);
        }

        if (pReader)
        {
            pReader->~ZGUIXMLReader2();
            ZUniMemory::Free(pReader);
        }
    }

    void ZMenuElements::DeleteMenuElements()
    {
        for (int32_t i = 0; i < m_iNumOfGUIBaseType; ++i)
            ZUniMemory::Free(m_aGUIBaseType[i].pGUIBase);

        m_iNumOfGUIBaseType = 0;
    }

    IGUIElement* ZMenuElements::CreateGUIElement(const char* pszType)
    {
        IGUIElement* pElement = nullptr;

        for (int32_t i = m_iNumOfMenuFactories - 1; i >= 0; --i)
        {
            pElement = m_apMenuFactory[i]->GUIElementFactory(pszType);
            if (pElement)
                break;
        }

        if (!pElement)
            return nullptr;

        const int32_t iIndex = AddGUIBase(pElement, EGUI_ELEMENT);
        ZASSERT(iIndex != -1);
        pElement->m_iIndex = iIndex;
        pElement->m_pColorSet = GetColorSet("Default");
        return pElement;
    }

    IWindowInterface* ZMenuElements::CreateGUIWindow(const char* pszType)
    {
        IWindowInterface* pWindow = nullptr;

        for (int32_t i = m_iNumOfMenuFactories - 1; i >= 0; --i)
        {
            pWindow = m_apMenuFactory[i]->WindowFactory(pszType, this);
            if (pWindow)
                break;
        }

        if (!pWindow)
            return nullptr;

        const int32_t iIndex = AddGUIBase(pWindow, EWINDOW);
        ZASSERT(iIndex != -1);
        return pWindow;
    }

    ZColorSet* ZMenuElements::CreateColorSet()
    {
        ZColorSet* pColorSet = static_cast<ZColorSet*>(ZUniMemory::Allocate(sizeof(ZColorSet)));
        if (pColorSet)
            new (pColorSet) ZColorSet();

        if (!pColorSet)
            return nullptr;

        const int32_t iIndex = AddGUIBase(pColorSet, ECOLOR_SET);
        ZASSERT(iIndex != -1);
        return pColorSet;
    }

    ZButtonGraphic* ZMenuElements::CreateButtonGraphic()
    {
        ZButtonGraphic* pButtonGraphic = static_cast<ZButtonGraphic*>(ZUniMemory::Allocate(sizeof(ZButtonGraphic)));
        if (pButtonGraphic)
            new (pButtonGraphic) ZButtonGraphic();

        if (!pButtonGraphic)
            return nullptr;

        const int32_t iIndex = AddGUIBase(pButtonGraphic, EBUTTON_GRAPHIC);
        ZASSERT(iIndex != -1);
        return pButtonGraphic;
    }

    ZGUIBase* ZMenuElements::CreateMisc(const char* pszTag)
    {
        if (strcmp(pszTag, "GraphicElement") == 0)
        {
            ZButtonGraphicPart* pGraphicPart = static_cast<ZButtonGraphicPart*>(ZUniMemory::Allocate(sizeof(ZButtonGraphicPart)));
            if (pGraphicPart)
                new (pGraphicPart) ZButtonGraphicPart();
            return pGraphicPart;
        }

        if (strcmp(pszTag, "Type") == 0)
        {
            ZButtonGraphicPartType* pGraphicPartType = static_cast<ZButtonGraphicPartType*>(ZUniMemory::Allocate(sizeof(ZButtonGraphicPartType)));
            if (pGraphicPartType)
                new (pGraphicPartType) ZButtonGraphicPartType();
            return pGraphicPartType;
        }

        if (strcmp(pszTag, "ToggleTextItem") == 0)
        {
            ZToggleTextItem* pToggleTextItem = static_cast<ZToggleTextItem*>(ZUniMemory::Allocate(sizeof(ZToggleTextItem)));
            if (pToggleTextItem)
                new (pToggleTextItem) ZToggleTextItem();
            return pToggleTextItem;
        }

        if (strcmp(pszTag, "Options") == 0 ||
            strcmp(pszTag, "Header") == 0 ||
            strcmp(pszTag, "Browser") == 0 ||
            strcmp(pszTag, "ItemLine") == 0 ||
            strcmp(pszTag, "Navigation") == 0)
        {
            ZGUIGroup* pGroup = static_cast<ZGUIGroup*>(ZUniMemory::Allocate(sizeof(ZGUIGroup)));
            if (pGroup)
                new (pGroup) ZGUIGroup();
            return pGroup;
        }

        return nullptr;
    }

    ZGUIBase* ZMenuElements::GetGUIBase(const char* pszName, EGUIBaseType eType)
    {
        if (m_iNumOfGUIBaseType <= 0)
            return nullptr;

        for (int32_t i = 0; i < m_iNumOfGUIBaseType; ++i)
        {
            if (m_aGUIBaseType[i].eType == eType &&
                strcmp(m_aGUIBaseType[i].pGUIBase->m_szName, pszName) == 0)
            {
                return m_aGUIBaseType[i].pGUIBase;
            }
        }

        return nullptr;
    }

    int ZMenuElements::AddGUIBase(ZGUIBase* pGUIBase, EGUIBaseType eType)
    {
        ZASSERT(m_iNumOfGUIBaseType < NUM_OF_GUIBASETYPE);
        m_aGUIBaseType[m_iNumOfGUIBaseType].pGUIBase = pGUIBase;
        m_aGUIBaseType[m_iNumOfGUIBaseType].eType = eType;
        return m_iNumOfGUIBaseType++;
    }
}
