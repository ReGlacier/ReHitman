#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Draw/ZDrawSurface.h>
#include <Glacier/GlacierFWD.h>
#include <cstdint>


namespace Glacier
{
    struct IView
    {
        virtual ZRender* Render() = 0;
        virtual void Init() = 0;
        virtual void End() = 0;
        virtual const uint32_t* Viewport() = 0;
        virtual void EnablePostfilter() = 0;
        virtual void RemoveCameras() = 0;
        virtual void SetViewport(const uint32_t* vp) = 0;
        virtual void SetOverrideScissor(const uint32_t*) = 0;
        virtual void AddCamera(ZCAMERA*) = 0;
        virtual void RemoveCamera(ZCAMERA*) = 0;
        virtual ZCAMERA* GetCamera(int) = 0;
        virtual void CreateSurface(ZDrawSurface::TARGET) = 0;
        virtual void GetDrawInfo(ZOldDrawInfo*) = 0;
        virtual bool IsCameraInView(ZBaseGeom*) = 0;
    };
}