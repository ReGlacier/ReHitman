#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Debug/Fwd.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZRuntimeComponentBase.h>


namespace Glacier
{
    class ZDrawDebugController : public ZComponentSingleton<ZDrawDebugController, ZRuntimeComponentBase>
    {
    public:
        // constants
        static constexpr int MAX_NUM_OBJECTS = 256;

        // vtbl
        ~ZDrawDebugController() override;

        // methods
        ZDrawDebugController();
        void Add(ZDrawDebugObject* pObject);
        void Remove(ZDrawDebugObject* pObject);
        void Draw(ZDrawDebugRender* pRender);
        void DrawFullScreen(ZDrawDebugRender* pRender);
        void FrameUpdate(float fDt);

        // members
        ZDrawDebugObject* m_apObjects[MAX_NUM_OBJECTS];
        uint32_t m_lNumObjects = 0;
    };
}