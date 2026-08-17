#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Debug/Fwd.h>
#include <Glacier/Render/Debug/ZDebugFrame.h>
#include <Glacier/Render/Debug/ZSimpleDebugMenu.h>


namespace Glacier
{
    class ZDrawDebugText : public ZDebugFrame
    {
    public:
        // types
        struct ZMenu
        {
            const char* m_pName;
            void(*m_CallBack)(const void*, bool);
            const void* m_pData;
            ZSimpleDebugMenu* m_pMenu;
            int8_t m_iSortOrder;
        };

        // vtbl | TODO: Finish me

        // methods | TODO: Finish me

        // members
        bool m_bInitialized;
        bool m_bSelectorMenu;
        bool m_bLocked;
        
        uint32_t m_lSelectedIndex;
        ZStackArray<16, ZDrawDebugText::ZMenu> m_Menus;
        ZDrawDebugText::ZMenu* m_pActiveMenu;
        ZDrawDebugRender* m_pRender;
    };
}