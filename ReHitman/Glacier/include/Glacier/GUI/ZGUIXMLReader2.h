#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/ZGUIBase.h>
#include <Glacier/ZSTL/SimpleXML.h>
#include <Glacier/ZSTL/ZStaticVector.h>


namespace Glacier
{
    // fwds
    class ZMenuElements;

    class ZGUIXMLReader2 : public SimpleXML
    {
    public:
        // types
        class ZGUITemplate : public ZGUIBase
        {
        public:
            // types
            struct STagGUIBase
            {
                char m_szTagName[64];
                ZGUIBase* m_pElement;
            };

            // vtbl
            void readParams(const char** ppParams, ZMenuElements* pElems) override;
            void addElement(const char* pName, ZGUIBase* pEntry) override;

            // methods
            ZGUITemplate();

            bool ParamsOnly() const;
            const char* GetName() const;
            const char* GetScope() const;
            const char** GetParams() const;
            void addElementsToParent(ZGUIBase* pParent);
            void SetScope(const char* pszScope);
            const char* AddAttrToBuffer(const char* psAttr);

            // members
            char m_acAttrBuffer[1024];
            int m_iUsedAttrBuffer;
            const char* m_pszTemplateName;
            const char* m_pszTemplateScope;
            const char* m_atts[100];
            ZStaticVector<ZGUIXMLReader2::ZGUITemplate::STagGUIBase, 16> m_Elements;
        };

        // vtbl
        void startElement(const char* name, const char** attrs) override;
        void endElement(const char* name) override;
        virtual bool XMLParse(const char* pBuffer, ZMenuElements* pElems);

        // methods
        ZGUIXMLReader2();
        ZGUIBase* Factory(const char*, const char*);
        const char** CreateAtts(const char** attsDest, const char** attsSource, ZGUITemplate* pGUITemplate);
        ZGUITemplate* CreateTemplate();
        ZGUITemplate* GetTemplate(const char* pszName);

        // members
        ZMenuElements* m_pMenuElements;
        const char* m_pszPlatform;
        char* m_pszBuildTag;
        zstring m_strScope;
        bool m_bInclude;
        bool m_abIncludeStack[16];
        int m_iIncludeStackSize;
        int32_t m_iNumOfUsedTemplates;
        ZGUITemplate m_aGUITemplates[128];
        ZStaticVector<ZGUIBase*,128> m_ElementStack;
        int m_lPlatformCurrentLineNr;
    };
}
