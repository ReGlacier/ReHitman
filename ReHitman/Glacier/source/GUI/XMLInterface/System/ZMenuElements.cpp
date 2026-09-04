#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/XMLInterface/Elements/ZColorSet.h>
#include <Glacier/GUI/XMLInterface/Readers/ZGUIXMLReader2.h>
#include <Glacier/GUI/XMLInterface/System/IMenuFactoryInterface.h>
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

    IGUIElement* ZMenuElements::GetGUIElement(int iIndex)
    {
        if (iIndex >= m_iNumOfGUIBaseType)
            return nullptr;

        ZASSERT(m_aGUIBaseType[iIndex].eType == EGUI_ELEMENT);
        return static_cast<IGUIElement*>(m_aGUIBaseType[iIndex].pGUIBase);
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

    IWindowInterface* ZMenuElements::GetWindow(const char* pszName)
    {
        ZGUIBase* pBase = GetGUIBase(pszName, EWINDOW);
        return pBase ? static_cast<IWindowInterface*>(pBase) : nullptr;
    }
}
