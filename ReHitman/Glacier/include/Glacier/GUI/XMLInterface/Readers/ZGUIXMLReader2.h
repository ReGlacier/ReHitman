#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/ZSTL/SimpleXML.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZSTL/ZStaticVector.h>


namespace Glacier
{
    // fwds
    class ZMenuElements;

    class ZGUIXMLReader2 : public SimpleXML
    {
    public:
        // constants
        static constexpr int INCLUDE_STACK_SIZE = 16;
        static constexpr int NUM_OF_TEMPLATES = 128;

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
            const char* const* GetParams() const;
            void addElementsToParent(ZGUIBase* pParent);
            const char* SetScope(const char* pszScope);
            const char* AddAttrToBuffer(const char* psAttr);

            static constexpr int NUM_OF_ATTRIBUTES = 100;
            static constexpr int SIZE_OF_ATTRIBUTE_BUFFER = 1024;
            static constexpr int SIZE_OF_VECTOR = 16;

            // members
            char m_acAttrBuffer[SIZE_OF_ATTRIBUTE_BUFFER]; // +0x4C - Verified by PC SetScope
            int32_t m_iUsedAttrBuffer; // +0x44C - Verified by PC SetScope
            const char* m_pszTemplateName; // +0x450 - Verified by PC readParams
            const char* m_pszTemplateScope; // +0x454 - Verified by PC readParams
            const char* m_atts[NUM_OF_ATTRIBUTES]; // +0x458 - Verified by PC readParams
            ZStaticVector<STagGUIBase, 16> m_Elements; // +0x5E8 - Verified by PC addElement
        };
        RE_VERIFY_SIZE(ZGUITemplate, 0xA2C); // Verified by PC ctor (element size 0xA2C)
        RE_VERIFY_OFFSET(ZGUITemplate, m_acAttrBuffer, 0x4C);
        RE_VERIFY_OFFSET(ZGUITemplate, m_iUsedAttrBuffer, 0x44C);
        RE_VERIFY_OFFSET(ZGUITemplate, m_pszTemplateName, 0x450);
        RE_VERIFY_OFFSET(ZGUITemplate, m_pszTemplateScope, 0x454);
        RE_VERIFY_OFFSET(ZGUITemplate, m_atts, 0x458);
        RE_VERIFY_OFFSET(ZGUITemplate, m_Elements, 0x5E8);

        // vtbl
        void startElement(const char* name, const char** attrs) override;
        void endElement(const char* name) override;

        // methods
        ZGUIXMLReader2();
        ~ZGUIXMLReader2();
        bool XMLParse(const char* pszFileName, ZMenuElements* pMenuElements);

    protected:
        ZGUIBase* Factory(const char* pszTag, const char* pszType);
        const char** CreateAtts(const char** ppDestAtts, const char** ppSrcAtts, ZGUITemplate* pTemplate);
        ZGUITemplate* CreateTemplate();
        ZGUITemplate* GetTemplate(const char* pszName);

    public:
        // members
        ZMenuElements* m_pMenuElements; // +0x1AC - Verified by PC ctor
        const char* m_pszPlatform; // +0x1B0 - Verified by PC XMLParse
        char* m_pszBuildTag; // +0x1B4 - Verified by PC XMLParse
        zstring m_strScope; // +0x1B8 - Verified by PC ctor
        bool m_bInclude; // +0x1C4 - Verified by PC ctor
        bool m_abIncludeStack[INCLUDE_STACK_SIZE]; // +0x1C5 - Verified by PC startElement
        int32_t m_iIncludeStackSize; // +0x1D8 - Verified by PC startElement
        int32_t m_iNumOfUsedTemplates; // +0x1DC - Verified by PC ctor
        ZGUITemplate m_aGUITemplates[NUM_OF_TEMPLATES]; // +0x1E0 - Verified by PC ctor
        ZStaticVector<ZGUIBase*, 128> m_ElementStack; // +0x517E0 - Verified by PC startElement
        int32_t m_lPlatformCurrentLineNr; // +0x519E4
    };
    RE_VERIFY_SIZE(ZGUIXMLReader2, 0x519E8); // Verified by PC alloc
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_pMenuElements, 0x1AC);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_pszPlatform, 0x1B0);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_pszBuildTag, 0x1B4);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_strScope, 0x1B8);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_bInclude, 0x1C4);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_abIncludeStack, 0x1C5);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_iIncludeStackSize, 0x1D8);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_iNumOfUsedTemplates, 0x1DC);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_aGUITemplates, 0x1E0);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_ElementStack, 0x517E0);
    RE_VERIFY_OFFSET(ZGUIXMLReader2, m_lPlatformCurrentLineNr, 0x519E4);
}
