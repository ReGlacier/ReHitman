#pragma once

#include <Glacier/ZMath.h>

namespace Hitman::BloodMoney
{
    class ZHM3MenuElements;

    class ZGUIBase
    {
    public:
        char m_name[64];	//0x0004
        Glacier::ZVector2 m_position; //0x0044

        virtual void readParams(const char**, ZHM3MenuElements*);
        virtual void addElement(const char*, ZGUIBase*);
    };
}