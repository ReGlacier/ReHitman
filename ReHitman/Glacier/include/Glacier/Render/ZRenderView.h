#pragma once

#include <Glacier/Geom/ZBaseGeom.h> // ZBaseGeom
#include <Glacier/Render/Draw/ZDrawSurface.h>
#include <Glacier/Render/IView.h>
#include <cstdint>


namespace Glacier
{
    struct ZRenderViewBase : public IView
    {};

    struct ZRenderView : public ZRenderViewBase
    {};
}