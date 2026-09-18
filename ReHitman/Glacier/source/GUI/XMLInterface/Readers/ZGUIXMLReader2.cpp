#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>
#include <Glacier/GUI/XMLInterface/Elements/ZColorSet.h>
#include <Glacier/GUI/XMLInterface/Readers/ZGUIXMLReader2.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZFilePath.h>
#include <malloc.h>
#include <cstdlib>
#include <cstring>


namespace Glacier
{
    namespace
    {
        const char* GetAttrCase(const char** ppAttrs, const char* pszName, bool bCaseInsensitive)
        {
            if (ppAttrs == nullptr || pszName == nullptr)
                return nullptr;

            for (int32_t i = 0; ppAttrs[i] != nullptr; i += 2)
            {
                if (bCaseInsensitive ? stricmp(ppAttrs[i], pszName) == 0 : strcmp(ppAttrs[i], pszName) == 0)
                    return ppAttrs[i + 1];
            }

            return nullptr;
        }

        bool ReadBool(bool* pOut, const char** ppAttrs, const char* pszName, bool bCaseInsensitive)
        {
            const char* pszAttr = GetAttrCase(ppAttrs, pszName, bCaseInsensitive);

            if (pszAttr == nullptr)
                return false;

            if (strcmp(pszAttr, "True") == 0)
            {
                *pOut = true;
                return true;
            }

            if (strcmp(pszAttr, "False") != 0)
                return false;

            *pOut = false;
            return true;
        }

        bool ReadInt(int32_t* pOut, const char** ppAttrs, const char* pszName)
        {
            const char* pszAttr = GetAttrCase(ppAttrs, pszName, false);

            if (pszAttr == nullptr)
                return false;

            *pOut = atoi(pszAttr);
            return true;
        }

        bool ReadText(zstring& rOut, const char* pszName, const char** ppAttrs)
        {
            const char* pszAttr = GetAttrCase(ppAttrs, pszName, false);

            if (pszAttr == nullptr)
                return false;

            zstring str(pszAttr);
            rOut = str;
            return true;
        }
    }

    // ZGUITemplate

    ZGUIXMLReader2::ZGUITemplate::ZGUITemplate()
    {
        m_iUsedAttrBuffer = 0;
        m_pszTemplateName = nullptr;
        m_pszTemplateScope = nullptr;
        for (int i = 0; i < 100; ++i)
            m_atts[i] = nullptr;
    }

    const char* ZGUIXMLReader2::ZGUITemplate::SetScope(const char* pszScope)
    {
        ZASSERT(m_iUsedAttrBuffer < SIZE_OF_ATTRIBUTE_BUFFER);
        char* pDst = m_acAttrBuffer + m_iUsedAttrBuffer;
        while (*pszScope)
        {
            ZASSERT(m_iUsedAttrBuffer < SIZE_OF_ATTRIBUTE_BUFFER);
            *pDst++ = *pszScope++;
            ++m_iUsedAttrBuffer;
        }
        ZASSERT(m_iUsedAttrBuffer < SIZE_OF_ATTRIBUTE_BUFFER);
        *pDst = 0;
        ++m_iUsedAttrBuffer;
        return pDst;
    }

    const char* ZGUIXMLReader2::ZGUITemplate::AddAttrToBuffer(const char* pszAttr)
    {
        char* pDst = m_acAttrBuffer + m_iUsedAttrBuffer;

        for (int32_t i = 0; pszAttr[i]; ++i)
        {
            ZASSERT(m_iUsedAttrBuffer < SIZE_OF_ATTRIBUTE_BUFFER);
            m_acAttrBuffer[m_iUsedAttrBuffer] = pszAttr[i];
            ++m_iUsedAttrBuffer;
        }

        ZASSERT(m_iUsedAttrBuffer < SIZE_OF_ATTRIBUTE_BUFFER);
        m_acAttrBuffer[m_iUsedAttrBuffer] = 0;
        ++m_iUsedAttrBuffer;
        return pDst;
    }

    bool ZGUIXMLReader2::ZGUITemplate::ParamsOnly() const
    {
        return m_Elements.size() == 0;
    }

    const char* ZGUIXMLReader2::ZGUITemplate::GetName() const
    {
        return m_pszTemplateName;
    }

    const char* ZGUIXMLReader2::ZGUITemplate::GetScope() const
    {
        return m_pszTemplateScope;
    }

    const char* const* ZGUIXMLReader2::ZGUITemplate::GetParams() const
    {
        return m_atts;
    }

    void ZGUIXMLReader2::ZGUITemplate::readParams(const char** ppAttrs, ZMenuElements* pElems)
    {
        int32_t iNumParams = 0;

        if (*ppAttrs)
        {
            const char** pCurrent = ppAttrs;
            while (*pCurrent)
            {
                ZASSERT(m_iUsedAttrBuffer < SIZE_OF_ATTRIBUTE_BUFFER);
                ZASSERT(iNumParams < NUM_OF_ATTRIBUTES);

                if (m_pszTemplateName || strcmp(*pCurrent, "Name") != 0)
                {
                    m_atts[iNumParams] = SetScope(*pCurrent);
                    m_atts[iNumParams + 1] = SetScope(pCurrent[1]);
                    iNumParams += 2;
                }
                else
                {
                    m_pszTemplateName = SetScope(pCurrent[1]);
                }

                pCurrent += 2;
            }

            ZASSERT(iNumParams < NUM_OF_ATTRIBUTES);
            m_atts[iNumParams] = nullptr;
        }
        else
        {
            m_atts[0] = nullptr;
        }
    }

    void ZGUIXMLReader2::ZGUITemplate::addElement(const char* pName, ZGUIBase* pEntry)
    {
        ZASSERT(strlen(pName) < 0x40);
        ZASSERT(m_Elements.size() < 16);

        STagGUIBase tag;
        strcpy(tag.m_szTagName, pName);
        tag.m_pElement = pEntry;
        m_Elements.push_back(tag);
    }

    void ZGUIXMLReader2::ZGUITemplate::addElementsToParent(ZGUIBase* pParent)
    {
        for (uint32_t i = 0; i < m_Elements.size(); ++i)
        {
            if (m_Elements[i].m_pElement)
                pParent->addElement(m_Elements[i].m_szTagName, m_Elements[i].m_pElement);
        }
    }

    // ZGUIXMLReader2

    ZGUIXMLReader2::ZGUIXMLReader2()
        : SimpleXML()
    {
        m_pMenuElements = nullptr;
        m_pszPlatform = nullptr;
        m_pszBuildTag = nullptr;
        m_bInclude = true;
        m_iIncludeStackSize = 0;
        m_iNumOfUsedTemplates = 0;
        m_lPlatformCurrentLineNr = 0;
    }

    ZGUIXMLReader2::~ZGUIXMLReader2()
    {
        // zstring destructor handles m_strScope cleanup
    }

    bool ZGUIXMLReader2::XMLParse(const char* pszFileName, ZMenuElements* pMenuElements)
    {
        int32_t iFileSize = g_pSysFile->GetSize(pszFileName, false);
        if (iFileSize <= 0)
            return false;

        ZFilePath path(pszFileName);
        char acScope[264];
        ZFilePath noExt(path.AsChar());
        const char* pszDot = strchr(path.AsChar(), '.');
        if (pszDot)
        {
            const char* pStart = path.AsChar();
            uint32_t iLen = static_cast<uint32_t>(pszDot - pStart);
            if (iLen >= sizeof(acScope))
                iLen = sizeof(acScope) - 1;
            memcpy(acScope, pStart, iLen);
            acScope[iLen] = 0;
            noExt = acScope;
        }
        m_strScope = noExt.AsChar();

        m_pszPlatform = g_pSysInterface->m_bUseGameController ? "PS2" : "PC";
        ZSysInterface::GetOption("BuildTag", &m_pszBuildTag);
        static char acDefaultBuildTag[] = "Default";
        if (m_pszBuildTag == nullptr)
            m_pszBuildTag = acDefaultBuildTag;

        m_pMenuElements = pMenuElements;
        m_bInclude = true;

        char* pFileContents = static_cast<char*>(alloca(iFileSize));
        g_pSysFile->Load(pszFileName, pFileContents, iFileSize, 0, false);
        XML_Parse(pFileContents, iFileSize, 1);
        return true;
    }

    void ZGUIXMLReader2::startElement(const char* pszTag, const char** ppAttrs)
    {
        if (strcmp(pszTag, "Platform") == 0 || (m_pszBuildTag && strcmp(pszTag, "BuildTag") == 0))
        {
            ZASSERT(m_iIncludeStackSize < INCLUDE_STACK_SIZE);
            m_abIncludeStack[m_iIncludeStackSize++] = m_bInclude;

            bool bMatch = false;
            if (!ReadBool(&bMatch, ppAttrs, m_pszPlatform, true) && m_pszBuildTag)
                ReadBool(&bMatch, ppAttrs, m_pszBuildTag, false);

            m_bInclude = bMatch && m_bInclude;
        }
        else if (strcmp(pszTag, "MainIniOption") == 0)
        {
            ZASSERT(m_iIncludeStackSize < INCLUDE_STACK_SIZE);
            m_abIncludeStack[m_iIncludeStackSize++] = m_bInclude;

            int32_t iValue = 1;
            ReadInt(&iValue, ppAttrs, "Value");

            zstring strName;
            ReadText(strName, "Name", ppAttrs);

            char* pszOptionValue = nullptr;
            int32_t iOptionValue = 0;
            if (ZSysInterface::GetOption(strName.c_str(), &pszOptionValue))
            {
                iOptionValue = 1;
                if (pszOptionValue && strlen(pszOptionValue))
                    iOptionValue = atoi(pszOptionValue);
            }

            m_bInclude = (iValue == iOptionValue) && m_bInclude;
        }
        else if (m_bInclude)
        {
            const char* pszTemplateName = GetAttr(ppAttrs, "Template");
            ZGUITemplate* pTemplate = GetTemplate(pszTemplateName);

            const char* acAttsBuffer[100];
            const char** ppAtts = CreateAtts(acAttsBuffer, ppAttrs, pTemplate);

            if (pTemplate && !pTemplate->ParamsOnly())
            {
                if (m_ElementStack.size() > 0)
                    pTemplate->addElementsToParent(m_ElementStack[m_ElementStack.size() - 1]);

                m_ElementStack.push_back(nullptr);
            }
            else
            {
                const char* pszType = GetAttr(ppAtts, "Type");
                ZGUIBase* pElement = Factory(pszTag, pszType);

                if (m_ElementStack.size() < 128)
                    m_ElementStack.push_back(pElement);

                if (pElement)
                    pElement->readParams(ppAtts, m_pMenuElements);
            }
        }
    }

    void ZGUIXMLReader2::endElement(const char* pszTag)
    {
        if (strcmp(pszTag, "Platform") == 0 || (m_pszBuildTag && strcmp(pszTag, "BuildTag") == 0) || strcmp(pszTag, "MainIniOption") == 0)
        {
            --m_iIncludeStackSize;
            m_bInclude = m_abIncludeStack[m_iIncludeStackSize];
        }
        else if (m_bInclude)
        {
            ZASSERT(m_ElementStack.size() > 0);

            ZGUIBase* pElement = m_ElementStack[m_ElementStack.size() - 1];
            uint32_t iNewSize = m_ElementStack.pop_back();

            if (iNewSize > 1)
            {
                ZGUIBase* pParent = m_ElementStack[m_ElementStack.size() - 1];
                if (pParent && pElement)
                    pParent->addElement(pszTag, pElement);
            }
        }
    }

    ZGUIBase* ZGUIXMLReader2::Factory(const char* pszTag, const char* pszType)
    {
        if (strcmp(pszTag, "Template") == 0)
            return CreateTemplate();

        if (pszType && strcmp(pszTag, "GuiElement") == 0)
            return static_cast<ZGUIBase*>(m_pMenuElements->CreateGUIElement(pszType));

        if (strcmp(pszTag, "ColorSetDefinition") == 0)
            return static_cast<ZGUIBase*>(m_pMenuElements->CreateColorSet());

        if (strcmp(pszTag, "ButtonGraphicDefinition") == 0)
            return static_cast<ZGUIBase*>(m_pMenuElements->CreateButtonGraphic());

        if (strcmp(pszTag, "Window") != 0)
            return m_pMenuElements->CreateMisc(pszTag);

        const char* pWindowType = pszType ? pszType : "Standard";
        return static_cast<ZGUIBase*>(m_pMenuElements->CreateGUIWindow(pWindowType));
    }

    const char** ZGUIXMLReader2::CreateAtts(const char** ppDestAtts, const char** ppSrcAtts, ZGUITemplate* pTemplate)
    {
        if (pTemplate == nullptr)
            return ppSrcAtts;

        int32_t iCount = 0;
        const char** pSrc = ppSrcAtts;

        while (*pSrc)
        {
            ZASSERT(iCount < 100);
            ppDestAtts[iCount++] = *pSrc++;
            ppDestAtts[iCount++] = *pSrc++;
        }

        const char* const* pTemplateAtts = pTemplate->GetParams();
        while (*pTemplateAtts)
        {
            ZASSERT(iCount < 100);
            ppDestAtts[iCount++] = *pTemplateAtts++;
            ppDestAtts[iCount++] = *pTemplateAtts++;
        }

        ZASSERT(iCount < 100);
        ppDestAtts[iCount] = nullptr;
        return ppDestAtts;
    }

    ZGUIXMLReader2::ZGUITemplate* ZGUIXMLReader2::CreateTemplate()
    {
        if (m_iNumOfUsedTemplates >= NUM_OF_TEMPLATES)
            return nullptr;

        ZGUITemplate* pTemplate = &m_aGUITemplates[m_iNumOfUsedTemplates];
        ++m_iNumOfUsedTemplates;
        pTemplate->SetScope(m_strScope.c_str());
        return pTemplate;
    }

    ZGUIXMLReader2::ZGUITemplate* ZGUIXMLReader2::GetTemplate(const char* pszName)
    {
        zstring aParts[2];
        int32_t iNumParts = 0;

        if (pszName)
        {
            const char* pszPipe = strchr(pszName, '|');

            if (pszPipe)
            {
                aParts[0] = zstring(pszName, static_cast<uint32_t>(pszPipe - pszName));
                aParts[1] = zstring(pszPipe + 1);
                iNumParts = 2;
            }
            else
            {
                aParts[0] = zstring(pszName);
                iNumParts = 1;
            }
        }

        if (iNumParts <= 0)
            return nullptr;

        for (int32_t iPart = 0; iPart < iNumParts; ++iPart)
        {
            const char* pszPart = aParts[iPart].c_str();
            uint32_t iPartLen = aParts[iPart].length();

            const char* pszColon = nullptr;
            for (uint32_t i = 0; i < iPartLen; ++i)
            {
                if (pszPart[i] == ':')
                {
                    pszColon = &pszPart[i];
                    break;
                }
            }

            const char* pszScope = pszPart;
            uint32_t iScopeLen = iPartLen;
            const char* pszNameStart = pszPart;

            if (pszColon)
            {
                pszScope = pszColon + 1;
                while (*pszScope == ':')
                    ++pszScope;
                pszNameStart = pszScope;
                iScopeLen = static_cast<uint32_t>(pszColon - pszPart);
            }
            else
            {
                pszScope = m_strScope.c_str();
                iScopeLen = m_strScope.length();
            }

            for (int32_t iTemplate = 0; iTemplate < m_iNumOfUsedTemplates; ++iTemplate)
            {
                ZGUITemplate* pTemplate = &m_aGUITemplates[iTemplate];
                const char* pszTemplateName = pTemplate->GetName();

                if (pszTemplateName && strcmp(pszTemplateName, pszNameStart) == 0 && strncmp(pTemplate->GetScope(), pszScope, iScopeLen) == 0)
                    return pTemplate;
            }
        }

        return nullptr;
    }
}
