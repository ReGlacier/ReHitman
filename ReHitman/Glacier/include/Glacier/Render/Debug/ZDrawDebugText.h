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
            // methods
            ZMenu();
            ZMenu(const ZMenu& copy);
            ZMenu& operator=(const ZMenu& rhs);

            // members
            const char* m_pName;
            void(*m_CallBack)(const void*, bool);
            const void* m_pData;
            ZSimpleDebugMenu* m_pMenu;
            int8_t m_iSortOrder;
        };

        // vtbl
        virtual void Update(ZDrawDebugRender* pRender);
        virtual void TextPlot(uint32_t x, uint32_t y, const char* pText, uint32_t dwTextColor);
        virtual void TextPlotInvers(uint32_t x, uint32_t y, const char* pText, uint32_t dwTextColor, uint32_t dwBgColor);
        virtual void AddMenu(const char* pName, void(*CallBack)(const void*, bool), const void* pData);
        virtual void AddMenu(const char* pName, ZSimpleDebugMenu* pSimpleMenu);
        virtual void Init();
        virtual void End();
        virtual void Update();
        virtual void Lock();
        virtual void Unlock();

        // methods
        ZDrawDebugText();
        void ReturnToMainMenu();
        void DrawInfo();
        void DrawText(uint32_t x, uint32_t y, const char* pszText, uint32_t lColor);
        void DrawRect(uint32_t lPosX, int lPosY, int lSizeX, int lSizeY, uint32_t lColor);
        void DrawLine(float fStartX, float fStartY, float fEndX, float fEndY, uint32_t lColor);

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
