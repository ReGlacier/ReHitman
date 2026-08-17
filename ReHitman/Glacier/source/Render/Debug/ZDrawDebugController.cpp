#include <Glacier/Render/Debug/ZDrawDebugController.h>
#include <Glacier/Render/Debug/ZDrawDebugObject.h>
#include <Glacier/Render/Debug/ZDrawDebugRender.h>


namespace Glacier
{
    ZDrawDebugController::ZDrawDebugController() = default;
    ZDrawDebugController::~ZDrawDebugController() = default;

    void ZDrawDebugController::Add(ZDrawDebugObject* pObject)
    {
        ZASSERT(pObject);
        ZASSERT(m_lNumObjects + 1 < MAX_NUM_OBJECTS);

        m_apObjects[m_lNumObjects++] = pObject;
    }

    void ZDrawDebugController::Remove(ZDrawDebugObject* pObject)
    {
        for (int i = 0; i < m_lNumObjects; ++i)
        {
            if (m_apObjects[i] == pObject)
            {
                m_apObjects[i] = nullptr;
                --m_lNumObjects;
                return;
            }
        }
    }

    void ZDrawDebugController::Draw(ZDrawDebugRender* pRender)
    {
        for (int i = 0; i < m_lNumObjects; ++i)
        {
            pRender->xReset();
            m_apObjects[i]->Draw(pRender);
        }
    }

    void ZDrawDebugController::DrawFullScreen(ZDrawDebugRender* pRender)
    {
        for (int i = 0; i < m_lNumObjects; ++i)
        {
            pRender->xReset();
            m_apObjects[i]->DrawFullScreen(pRender);
        }
    }

    void ZDrawDebugController::FrameUpdate(float fDt)
    {
        for (int i = 0; i < m_lNumObjects; ++i)
        {
            m_apObjects[i]->FrameUpdate(fDt);
        }
    }

    template <>
    ZDrawDebugController* ZComponentSingleton<ZDrawDebugController, ZRuntimeComponentBase>::m_pInstance = nullptr;
}