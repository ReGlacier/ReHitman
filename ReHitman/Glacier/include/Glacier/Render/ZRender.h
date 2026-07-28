#pragma once

#include <Glacier/Geom/ZBaseGeom.h> // ZBaseGeom
#include <Glacier/Render/ZDrawSurface.h>

namespace Glacier
{
    struct ZRenderDrawBase;
    struct ZBoneModifyBase;
    struct SPrims;
    struct IView;
    class ZCAMERA;
    class REFTAB;
    class LINKSORTREFTAB;

    struct ZRender
    {
        // vtbl
        virtual ~ZRender();
        virtual void Close();
        virtual void CrashClose();
        virtual void SetWideScreen(bool);
        virtual void SetLetterBox(bool);
        virtual void SetSurroundGaming(bool);
        virtual bool ShadowMode();
        virtual void SetCopyToFrontAlpha(unsigned int);
        virtual ZRenderDrawBase *GetDrawBase();
        virtual void UnknownMethod(); // PC only
        virtual bool BeginScene();
        virtual bool EndScene();
        virtual void ForceAllLeave();
        virtual void SetLOD(unsigned __int8 *, unsigned int, unsigned int);
        virtual void AllocateDrawBuffers();
        virtual void FreeDrawBuffers();
        virtual void InvalidateDraw();
        virtual void FadeScreen(float, unsigned int);
        virtual unsigned int LockDrawBaseGeomsBuffer(void **);
        virtual void UnlockDrawBaseGeomsBuffer();
        virtual void Flip();
        virtual void CopyFrontToBack();
        virtual void GetClippedPrimTriangles(REFTAB *, unsigned int, float *, unsigned int, const float *, const float *, ZCAMERA *, ZBoneModifyBase *);
        virtual void DrawMemory(unsigned int);
        virtual void DrawMemory(const unsigned int *, const unsigned int);
        virtual void DrawDebugObjectPrim(const SPrims *, float *, float *, unsigned int, unsigned int);
        virtual void ToggleMode(int);
        virtual void ResetFadeingLights(ZLNKOBJ*);
        virtual void SetKeepInside(bool);
        virtual bool GetKeepInside();
        virtual void Dump(const char *);
        virtual LINKSORTREFTAB *GetCameraList();
        virtual IView *CreateView(unsigned int, ZDrawSurface::TARGET);
        virtual IView *FindView(unsigned int);
        virtual IView *GetViewByIndex(int);
        virtual bool RemoveView(IView *);
        virtual void AddCamera(ZCAMERA *, unsigned int, float);
        virtual void InitCamera(ZCAMERA *);
        virtual unsigned int GetCamera(int);
        virtual void RemoveCamera(ZCAMERA *, unsigned int);
        virtual void RemoveCameras();
        virtual bool ChkCamera(ZCAMERA *);
        virtual void SetCameraPrio(ZCAMERA *, float);
        virtual void Update();
        virtual void UpdateCameras();
        virtual void HandleShadows();
        virtual int GetSizeX();
        virtual int GetSizeY();
        virtual void SetSize(int, int);
        virtual float ScreenAspectXY();
        virtual float PixelAspectXY();
        virtual int GetTextSizeX();
        virtual int GetTextSizeY();
        virtual float GetLetterSizeX();
        virtual float GetLetterSizeY();
        virtual void SetFontSize(const float *);
        virtual void GetFontSize(float *);
        virtual void SetEnvironment(float, int);
        virtual void PlotStatCounters(int);
        virtual void GetMouse(float *);
        virtual void GetMouse3D(float *);
        virtual void GetMouseButtons(bool *, bool *, bool *);
        virtual void DrawMouse(int);
        virtual void UPlotF(int, int, const char *);
        virtual void UPlotFNxt(const char *);
        virtual void SetViewport(const float, const float, const float, const float);
        virtual void ClearViewport(const unsigned int, const unsigned int);
        virtual void SetState();
        virtual void ResetState();
        virtual void PushState();
        virtual void PopState();
        virtual void SetProjection(const float, const float, const float, const float, const float, const float, const bool);
        virtual void SetModelView(const float *, const float *, const float, const float);
        virtual void SetFog(const float, const float, const unsigned int);
        virtual const char *GetRenderName();
        virtual void ProgressBar(const float, const unsigned int, const unsigned int);
        virtual void RemoveBaseGeom(ZBaseGeom *);
        virtual void UpdateBaseGeom(ZBaseGeom *);
        virtual void ChangePrim(ZBaseGeom *, unsigned int);
        virtual void BeginShadows();
        virtual void AddShadow(ZBaseGeom *, ZBaseGeom *, ZBaseGeom *, ZBaseGeom **, unsigned int);
        virtual void EndShadows();
        virtual void SetEditCursor(int);
        virtual int GetEditCursor();
        virtual unsigned int SetMaxFrameInterval(unsigned int);
        virtual void UpdateShortCuts();

        // methods
        // members
        struct ZRender* SubChain; // +0x4
        struct ZRender* Nxt; // +0x8
        struct ZRender* Bef; // +0xC
        // TODO: Finish me
    };

    // TBC
}