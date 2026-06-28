#pragma once

#include <Glacier/Geom/ZEntityLocator.h> // ZBaseGeom
#include <Glacier/Render/ZDrawSurface.h>
#include <cstdint>


namespace Glacier
{
    struct ZRender;
    struct ZOldDrawInfo;
    class ZCAMERA;

    struct IView
    {
        virtual ~IView();
        virtual ZRender* Render();
        virtual void Init();
        virtual void End();
        virtual const uint32_t* Viewport();
        virtual void EnablePostfilter();
        virtual void RemoveCameras();
        virtual void SetViewport(const uint32_t* vp);
        virtual void SetOverrideScissor(const uint32_t*);
        virtual void AddCamera(ZCAMERA*);
        virtual void RemoveCamera(ZCAMERA*);
        virtual ZCAMERA* GetCamera(int);
        virtual void CreateSurface(ZDrawSurface::TARGET);
        virtual void GetDrawInfo(ZOldDrawInfo*);
        virtual bool IsCameraInView(ZBaseGeom*);
    };

    struct ZRenderViewBase : public IView
    {};

    struct ZRenderView : public ZRenderViewBase
    {};
}