#pragma once

#include <BloodMoney/Game/UI/ZCHAROBJ.h>

namespace Hitman::BloodMoney
{
    class ZLINEOBJ : public ZCHAROBJ
    {
    public:
        // vftable
        virtual void Wrap();
        virtual int GetCharWidth(const char**);
        virtual int GetStringWidth(const char* str, int length);
        virtual void SetWidth(int);
        virtual float GetWidth();

        // custom method
        static ZLINEOBJ* Create();

        // data (total size is 0xAC, base size is 0xA0)
        int m_fieldA0;
        int m_fieldA4;
        int m_fieldA8;
    };
}