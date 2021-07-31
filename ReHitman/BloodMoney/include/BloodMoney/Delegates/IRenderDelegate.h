#pragma once

#include <Glacier/ZCameraSpace.h>
#include <Glacier/ZDrawBuffer.h>

namespace Glacier {
    class ZGEOM;
}

namespace Hitman::BloodMoney
{
    class IRenderDelegate {
    public:
        virtual ~IRenderDelegate() noexcept = default;

        virtual void onPreDrawScene(Glacier::ZCameraSpace* pCameraSpace, Glacier::ZDrawBuffer* pDrawBuffer) = 0;
        virtual void onDrawGeom(Glacier::ZCameraSpace* pCameraSpace, Glacier::ZDrawBuffer* pDrawBuffer, Glacier::ZGEOM* pGeom) = 0;
        virtual void onPostDrawScene(Glacier::ZCameraSpace* pCameraSpace, Glacier::ZDrawBuffer* pDrawBuffer) = 0;
    };
}