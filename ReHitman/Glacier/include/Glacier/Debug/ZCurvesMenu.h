#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZCurvesMenu;
    
    class ZMenuItem
    {
    public:
        // members
        const char* pText;
        void(*pCallback)(void*);
    };

    class ZMenu
    {
    public:
        // members
        void(*pEnterMenuCallback)(ZCurvesMenu*);
        void(*pLeaveMenuCallback)(ZCurvesMenu*);
        void(*pMoveLeftCallback)(ZCurvesMenu*, bool);
        void(*pMoveRightCallback)(ZCurvesMenu*, bool);
        ZMenuItem* m_pItems;
        uint32_t m_lLastSelectedMenuItem;
    };
    
    class ZStoredMenu
    {
    public:
        // methods

        // members
        ZMenu* m_pMenu;
        uint32_t m_lSelectedMenuItem;
    };

    class ZCurvesMenu
    {
    public:
        // methods

        // members
    };
}