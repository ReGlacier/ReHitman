#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Hitman::BloodMoney
{
    class ZMenuElements;
    class ZWINGROUP;

    class ZGUIBase
    {
    public:
        // vtable
        virtual void readParams(const char**, ZMenuElements*);
        virtual void addElement(const char*, ZGUIBase*);

        // api
        void GetSize(ZWINGROUP* pGroup, Glacier::Vector2* pSize);
        void GetPos(Glacier::Vector2* pPos);

        // data (total size is 0x4C)
        char m_szName[0x40] { 0 };	//0x0004
        Glacier::ZVector2 m_v2Pos {}; //0x0044
    };
    RE_VERIFY_SIZE(ZGUIBase, 0x4C); // Verified
}