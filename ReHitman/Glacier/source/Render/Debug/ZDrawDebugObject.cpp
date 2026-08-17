#include <Glacier/Render/Debug/ZDrawDebugController.h>
#include <Glacier/Render/Debug/ZDrawDebugObject.h>
#include <Glacier/Component/ZComponentSingleton.h>


namespace Glacier
{
    ZDrawDebugObject::ZDrawDebugObject()
    {
        ZDrawDebugController::Instance().Add(this);
    }

    ZDrawDebugObject::~ZDrawDebugObject()
    {
        ZDrawDebugController::Instance().Remove(this);
    }

    void ZDrawDebugObject::Draw(ZDrawDebugRender*)
    {
        // Do nothing
    }

    void ZDrawDebugObject::DrawFullScreen(ZDrawDebugRender*)
    {
        // Do nothing
    }

    void ZDrawDebugObject::FrameUpdate(float fDt)
    {
        // Do nothing
    }

    void ZDrawDebugObject::SetColot(uint32_t lColor)
    {
        m_lColor = lColor;
    }
}