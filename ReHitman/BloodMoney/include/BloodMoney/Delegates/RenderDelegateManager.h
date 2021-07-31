#pragma once

#include <vector>
#include <BloodMoney/Delegates/IRenderDelegate.h>

namespace Hitman::BloodMoney {
    class RenderDelegateManager {
        RenderDelegateManager() = default;
    public:
        RenderDelegateManager(const RenderDelegateManager&) = delete;
        RenderDelegateManager(RenderDelegateManager&&) = delete;
        RenderDelegateManager& operator=(const RenderDelegateManager&) = delete;
        RenderDelegateManager& operator=(RenderDelegateManager&&) = delete;

        static RenderDelegateManager& getInstance();

        void registerDelegate(IRenderDelegate* pRenderDelegate);
        void unregisterDelegate(IRenderDelegate* pRenderDelegate);

        void beginSceneDraw(Glacier::ZCameraSpace* pCameraSpace, Glacier::ZDrawBuffer* pDrawBuffer);
        void endSceneDraw(Glacier::ZCameraSpace* pCameraSpace, Glacier::ZDrawBuffer* pDrawBuffer);
        void drawGeom(Glacier::ZCameraSpace* pCameraSpace, Glacier::ZDrawBuffer* pDrawBuffer, Glacier::ZGEOM* pGeom);
    private:
        std::vector<IRenderDelegate*> m_vDelegates;
    };
}