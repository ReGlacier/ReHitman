#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
    struct SimpleXML
    {
        // types
        enum Types : int
        {
            XML_START = 0x0,
            XML_END = 0x1,
            XML_STARTEND = 0x2,
            XML_COMMENT = 0x3,
            XML_ERROR = 0x4,
            XML_ENDOFDATA = 0x5,
        };

        enum
        {
            NATTRIBUTES = 0x64
        };

        // vtbl
        virtual void startElement(const char* name, const char** attrs);
        virtual void endElement(const char* name);

        // methods
        SimpleXML();

        int XML_Parse(char* buffer, int length, int isFinal);
        const char* GetAttr(const char** attrs, const char* name);
        int getElement();

        // data
        char* m_pStart;
        char* m_pEnd;
        char* m_p;
        int m_Len;
        char* m_pTag;
        int m_nAttributes;
        char* m_Attributes[NATTRIBUTES];

    private: // private stuff
        int printError(const char* psErrorMessage);
    };
    RE_VERIFY_SIZE(SimpleXML, 0x1AC);
}