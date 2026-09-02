#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZLINEOBJ : public ZCHAROBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZLINEOBJ, 0x200038u);

        // vftable
        virtual void Wrap();
        virtual int GetCharWidth(const char**);
        virtual int GetStringWidth(const char* str, int length);
        virtual void SetWidth(int);
        virtual float GetWidth();

        // custom method
        static ZLINEOBJ* Create();

        // data (total size is 0xAC, base size is 0xA0)
        char* m_szText;
        int32_t m_iWidth;
        int8_t m_iNumberOfLines;
    };
    RE_VERIFY_SIZE(ZLINEOBJ, 0xAC);
}
