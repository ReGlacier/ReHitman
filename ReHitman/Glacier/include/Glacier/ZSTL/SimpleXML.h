#pragma once


namespace Glacier
{
    struct SimpleXML
    {
        // vtbl
        virtual void startElement();
        virtual void endElement();

        // data
        char* m_pStart;
        char* m_pEnd;
        char* m_p;
        int m_Len;
        char* m_pTag;
        int m_nAttributes;
        char* m_Attributes[100];
    };
}