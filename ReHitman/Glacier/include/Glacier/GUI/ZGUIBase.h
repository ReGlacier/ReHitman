#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    // fwds
    class ZMenuElements;
    class ZWINGROUP;


    class ZGUIBase
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems);
        virtual void addElement(const char* pName, ZGUIBase* pEntry);

        // methods
        void GetSize(ZWINGROUP* pGroup, Glacier::Vector2* pSize);
        void GetPos(Glacier::Vector2* pPos);

        // members
        char m_szName[0x40] { 0 };	//0x0004
        Glacier::ZVector2 m_v2Pos {}; //0x0044
    };
    RE_VERIFY_SIZE(ZGUIBase, 0x4C); // Verified
}
