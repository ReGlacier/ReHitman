#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZSimpleDebugMenu;

    class ZSimpleMenuItem
    {
    public:
        char* pText;
        void(*pCallback)(ZSimpleDebugMenu*);
    };

    class ZSimpleMenu
    {
    public:
        void (*pEnterMenuCallback)(ZSimpleDebugMenu *);
        void (*pLeaveMenuCallback)(ZSimpleDebugMenu *);
        void (*pMoveLeftCallback)(ZSimpleDebugMenu *, bool);
        void (*pMoveRightCallback)(ZSimpleDebugMenu *, bool);
        ZSimpleMenuItem *pItems;
    };


    class ZStoredSimpleMenu
    {
    public:
        // methods

        // members
        ZSimpleMenu* m_pMenu;
        uint32_t m_lSelectedMenuItem;
    };

    class ZSimpleDebugMenu
    {
    public:
        // vtbl
        virtual void Init();
        virtual void Update();
        virtual void BeginEdit();
        virtual void EndEdit();
        virtual bool LeaveMenu();
        virtual void EnterMenu(ZSimpleMenu* pMenu);
        virtual void DrawAlways();
        virtual void Draw();

        // methods
        ZSimpleDebugMenu();
        void CountMenuItems();
        uint32_t CalcMenuSizeX() const;
        uint32_t CalcMenuSizeY() const;
        bool KeyRepeatWait(bool bRepeat);

        // members
        uint32_t m_iNumberItems; // +0x4
        bool m_bEditEnabled; // +0x8
        RE_ADD_PADDING(3);
        uint32_t m_lSelectedMenuItem; // +0xC
        ZSimpleMenu* m_pCurrentMenu; // +0x10
        ZStackArray<32, ZStoredSimpleMenu> m_LastMenu; // +0x14
    };
}
