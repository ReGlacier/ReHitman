#pragma once

#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Render/Draw/ZDrawSurface.h>
#include <Glacier/ZSTL/LINKSORTREFTAB.h>
#include <cstdint>

namespace Glacier
{
    struct ZRenderDrawBase;
    struct ZBoneModifyBase;
    struct SPrims;
    struct IView;
    class ZCAMERA;
    class REFTAB;
    class LINKSORTREFTAB;
    class ZDrawDebugRender;
    class ZMemColorsDisplay;

    // Per-frame statistics block inside ZRender. Layout recovered from the shared
    // engine source (Z:\code\engine\drawing\renderbase.cpp) and PS2 startup.cpp;
    // confirmed by ZRenderWintelD3D::Update (PC) zeroing the whole 0x30-byte block.
    struct ZRStatCounters
    {
        // members
        uint32_t m_lTriangleCount;        // +0x0
        uint32_t m_lTriangleCountBones;   // +0x4
        uint32_t m_lStripCount;           // +0x8
        uint32_t m_lPrimitiveCount;       // +0xC
        uint32_t m_lSubPrimitiveCount;    // +0x10
        uint32_t m_lTextureCount;         // +0x14
        uint32_t m_lTextureSize;          // +0x18
        uint32_t m_lBoneCount;            // +0x1C
        uint32_t m_lSubBoneCount;         // +0x20
        uint32_t m_lDegenerateCount;      // +0x24
        uint32_t m_lDegenerateCountBones; // +0x28
        uint32_t m_lMaterialCount;        // +0x2C
    };
    RE_VERIFY_SIZE(ZRStatCounters, 0x30);

    class ZRender
    {
    public:
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
        virtual void ColorFill(); // slot 9 (PC-only; base is an empty stub, ZRenderWintelD3D implements it)
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
        virtual void Flip() = 0; // pure virtual (PC _purecall, slot 20)
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
        virtual void DrawMouse(int) = 0; // pure virtual (PC _purecall, slot 62)
        virtual void UPlotF(int PosX, int PosY, const char* Text);
        virtual void UPlotFNxt(const char* Text);
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
        ZRender(int hInstance, void* hParentWnd); // PC 0x00469780
        void Initialize(int hInstance, void* hParentWnd); // PC 0x004684C0 (called by the ctor)
        void AddCameraToView(ZCAMERA* pCamera, uint32_t lViewId); // PC 0x00468A00 (non-virtual helper, used by AddCamera/AllocateDrawBuffers)

        // members
        struct ZRender* SubChain; // +0x4
        struct ZRender* Nxt; // +0x8
        struct ZRender* Bef; // +0xC
        uint8_t m_aLODLookupTable[0x100]; // +0x10. Maps clamped scaled distance index (0-255) to LOD level; filled by ZRender::SetLOD (background LOD table, 7 levels)
        uint8_t m_LODTableCharacters[0x100]; // +0x110. Character LOD table; filled by ZRender::SetLOD (6 levels, "DisableCharacterLOD" option)
        bool m_bFlipDisabled; // +0x210. Gates the flip path in ZRenderWintelD3D::Update (checked at +0x210)
        bool m_bEnable4_3CutOff; // +0x211
        bool m_bForceLightUpdate; // +0x212
        RE_ADD_PADDING(1); // +0x213
        float m_fLightMultiplier; // +0x214
        int32_t m_FPS; // +0x218. Set to display refresh rate / 60 by default
        bool m_bShadowMode; // +0x21C. Returned by ShadowMode()
        RE_ADD_PADDING(3); // +0x21D
        void* m_pLockDrawBaseMemory; // +0x220. LockDrawBaseGeomsBuffer / UnlockDrawBaseGeomsBuffer
        int32_t m_lLockDrawBaseSize; // +0x224
        ZCAMERA* m_pCamera; // +0x228
        float m_fCameraFarInv; // +0x22C
        uint32_t m_lEnvironmentLightDrawId; // +0x230
        bool m_bInitialized; // +0x234
        RE_ADD_PADDING(3); // +0x235
        ZRStatCounters m_StatCounters; // +0x238 (0x30 bytes)
        uint32_t m_lTriangleCount; // +0x268
        uint32_t m_lTriangleCountBones; // +0x26C
        uint32_t m_lStripCount; // +0x270
        uint32_t m_lPrimitiveCount; // +0x274
        uint32_t m_lSpriteCount; // +0x278
        uint32_t m_lSubPrimitiveCount; // +0x27C
        uint32_t m_lTextureCount; // +0x280
        uint32_t m_lTextureSize; // +0x284
        uint32_t m_lBoneCount; // +0x288
        uint32_t m_lSubBoneCount; // +0x28C
        uint32_t m_lMaxFrameInterval; // +0x290. SetMaxFrameInterval
        float m_MouseX; // +0x294
        float m_MouseY; // +0x298
        float m_MouseZ; // +0x29C
        int32_t m_ScrMouseX; // +0x2A0
        int32_t m_ScrMouseY; // +0x2A4
        bool m_bLeftMouseButtonDown; // +0x2A8
        bool m_bMiddleMouseButtonDown; // +0x2A9
        bool m_bRightMouseButtonDown; // +0x2AA
        RE_ADD_PADDING(1); // +0x2AB
        float m_fShowEditCursor; // +0x2AC. SetEditCursor / GetEditCursor
        int32_t m_NxtLine; // +0x2B0
        int32_t m_RemNxtLine; // +0x2B4
        bool m_bConsoleKey; // +0x2B8
        bool m_bWideScreen; // +0x2B9. SetWideScreen
        bool m_bLetterBox; // +0x2BA. SetLetterBox
        bool m_bSurroundGaming; // +0x2BB. SetSurroundGaming
        int32_t m_Size[2]; // +0x2BC. GetSizeX/GetSizeY, SetSize
        LINKSORTREFTAB m_Camera; // +0x2C4 (0x2C bytes). GetCameraList / GetCamera / AddCamera
        int32_t m_lLockCount; // +0x2F0
        float m_fFontSize[2]; // +0x2F4. SetFontSize / GetFontSize
        ZRenderDrawBase* m_pRenderDraw; // +0x2FC. GetDrawBase
        uint32_t m_lInCutSequence; // +0x300. Cut-sequence nesting counter (SetInCutSequence)
        bool m_bUseSWLight; // +0x304
        bool m_bMovieRunning; // +0x305
        RE_ADD_PADDING(2); // +0x306
        ZDrawDebugRender* m_pDrawDebugRender; // +0x308
        ZMemColorsDisplay* m_pMemColorsDisplay; // +0x30C
        float m_MC_Time; // +0x310
        bool m_MC_SelectedStatus; // +0x314
        RE_ADD_PADDING(3); // +0x315
        int32_t m_MC_SelectedBlocksUsed; // +0x318
    };

    RE_VERIFY_OFFSET(ZRender, m_aLODLookupTable, 0x10); // Verified by ZRenderEntry::CalcLODMask (PC)
    RE_VERIFY_OFFSET(ZRender, m_LODTableCharacters, 0x110); // Verified by ZRender::Initialize SetLOD(&this+0x110) (PC)
    RE_VERIFY_OFFSET(ZRender, m_bFlipDisabled, 0x210); // Verified by ZRenderWintelD3D::Update (PC, cmp [this+0x210])
    RE_VERIFY_OFFSET(ZRender, m_fLightMultiplier, 0x214); // Verified by ZRender::Initialize (PC)
    RE_VERIFY_OFFSET(ZRender, m_bShadowMode, 0x21C); // Verified by ZRender::ShadowMode (PC)
    RE_VERIFY_OFFSET(ZRender, m_pLockDrawBaseMemory, 0x220); // Verified by Lock/UnlockDrawBaseGeomsBuffer (PC)
    RE_VERIFY_OFFSET(ZRender, m_lLockDrawBaseSize, 0x224); // Verified by Lock/UnlockDrawBaseGeomsBuffer (PC)
    RE_VERIFY_OFFSET(ZRender, m_bInitialized, 0x234); // Verified by ZRender::Initialize / Close / Update (PC)
    RE_VERIFY_OFFSET(ZRender, m_StatCounters, 0x238); // Verified by ZRenderWintelD3D::Update zeroing +0x238..+0x264 (PC)
    RE_VERIFY_OFFSET(ZRender, m_lMaxFrameInterval, 0x290); // Verified by SetMaxFrameInterval (PC)
    RE_VERIFY_OFFSET(ZRender, m_ScrMouseX, 0x2A0); // Verified by ZRender::Initialize (PC)
    RE_VERIFY_OFFSET(ZRender, m_bLeftMouseButtonDown, 0x2A8); // Verified by ZRender::GetMouseButtons (PC)
    RE_VERIFY_OFFSET(ZRender, m_fShowEditCursor, 0x2AC); // Verified by SetEditCursor / GetEditCursor (PC)
    RE_VERIFY_OFFSET(ZRender, m_NxtLine, 0x2B0); // Verified by ZRenderWintelD3D::Update (PC)
    RE_VERIFY_OFFSET(ZRender, m_bWideScreen, 0x2B9); // Verified by SetWideScreen (PC)
    RE_VERIFY_OFFSET(ZRender, m_Size, 0x2BC); // Verified by GetSizeX/GetSizeY/SetSize (PC)
    RE_VERIFY_OFFSET(ZRender, m_Camera, 0x2C4); // Verified by GetCameraList / GetCamera (PC)
    RE_VERIFY_OFFSET(ZRender, m_fFontSize, 0x2F4); // Verified by SetFontSize / GetFontSize (PC)
    RE_VERIFY_OFFSET(ZRender, m_pRenderDraw, 0x2FC); // Verified by GetDrawBase / UpdateCameras (PC)
    RE_VERIFY_OFFSET(ZRender, m_lInCutSequence, 0x300); // Verified by ZRender::Initialize (PC) and SetInCutSequence counter (PS2/KL2)
    RE_VERIFY_OFFSET(ZRender, m_bUseSWLight, 0x304); // Verified by SetUseSWLight (PS2)
    RE_VERIFY_OFFSET(ZRender, m_bMovieRunning, 0x305); // Verified by SetFullScreenMovieRunning (PS2)
    RE_VERIFY_OFFSET(ZRender, m_pDrawDebugRender, 0x308); // Verified by ZRender::Ctor (PC)
    RE_VERIFY_OFFSET(ZRender, m_pMemColorsDisplay, 0x30C); // Verified by ZRender::Initialize (PC) and ZMemColorsDisplay::FrameUpdate (PS2)
    RE_VERIFY_OFFSET(ZRender, m_MC_SelectedBlocksUsed, 0x318);
    RE_VERIFY_SIZE(ZRender, 0x31C); // Base ZRender only; ZRenderX86 continues at +0x31C (hInstance)

    // TBC
}
