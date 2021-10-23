#pragma once

#include <Glacier/ZSTL/ZMath.h>

namespace Hitman::BloodMoney
{
    class ZHM3MenuElements;
    class ZWINGROUP;

    class ZGUIBase
    {
    public:
        // vtable
        virtual void readParams(const char**, ZHM3MenuElements*);
        virtual void addElement(const char*, ZGUIBase*);

        // api
        void GetSize(ZWINGROUP* pGroup, Glacier::Vector2* pSize);

        // data (total size is 0x4C)
        char m_name[0x40] { 0 };	//0x0004
        Glacier::ZVector2 m_position {}; //0x0044
    };
}