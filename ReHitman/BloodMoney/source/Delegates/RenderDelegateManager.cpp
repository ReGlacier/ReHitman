#include <BloodMoney/Delegates/RenderDelegateManager.h>
#include <algorithm>

namespace Hitman::BloodMoney
{
    RenderDelegateManager& RenderDelegateManager::getInstance() {
        static RenderDelegateManager g_manager;
        return g_manager;
    }

    void RenderDelegateManager::registerDelegate(IRenderDelegate* pRenderDelegate) {
        if (!pRenderDelegate) {
            return;
        }

        m_vDelegates.push_back(pRenderDelegate);
    }

    void RenderDelegateManager::unregisterDelegate(IRenderDelegate* pRenderDelegate) {
        auto it = std::remove_if(std::begin(m_vDelegates), std::end(m_vDelegates), [pRenderDelegate](const IRenderDelegate* pDelegate) -> bool {
            return pDelegate == pRenderDelegate;
        });

        m_vDelegates.erase(it, std::end(m_vDelegates));
    }

    void RenderDelegateManager::beginSceneDraw(Glacier::ZCameraSpace* pCameraSpace,
                                               Glacier::ZDrawBuffer* pDrawBuffer)
    {
        for (const auto& pDelegate : m_vDelegates) {
            pDelegate->onPreDrawScene(pCameraSpace, pDrawBuffer);
        }
    }

    void RenderDelegateManager::endSceneDraw(Glacier::ZCameraSpace* pCameraSpace, Glacier::ZDrawBuffer* pDrawBuffer)
    {
        for (const auto& pDelegate : m_vDelegates) {
            pDelegate->onPostDrawScene(pCameraSpace, pDrawBuffer);
        }
    }

    void RenderDelegateManager::drawGeom(Glacier::ZCameraSpace* pCameraSpace,
                                         Glacier::ZDrawBuffer* pDrawBuffer,
                                         Glacier::ZGEOM* pGeom)
    {
        for (const auto& pDelegate : m_vDelegates) {
            pDelegate->onDrawGeom(pCameraSpace, pDrawBuffer, pGeom);
        }
    }
}