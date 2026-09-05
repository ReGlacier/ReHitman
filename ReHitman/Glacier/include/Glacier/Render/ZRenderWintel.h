#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderX86.h>
#include <Glacier/Render/Common.h>

#define NOMINMAX
#include <Windows.h>


namespace Glacier
{
    class ZCAMERA;
    class ZBoneModifyBase;
    struct REFTAB;

    // PC vtable 0x0076320C (156 slots). ZRenderWintel is the base "Windows" render:
    // it owns the window/message handling and resolution management. The heavy
    // D3D work lives in ZRenderWintelD3D (derived).
    class ZRenderWintel : public ZRenderX86
    {
    public:
        // vtbl (overrides of base slots)
        ~ZRenderWintel() override;
        void GetClippedPrimTriangles(REFTAB*, unsigned int, float*, unsigned int, const float*, const float*, ZCAMERA*, ZBoneModifyBase*) override; // PC 0x00486380
        void SetKeepInside(bool bKeepInside) override;   // PC 0x004844D0 (clip cursor)
        bool GetKeepInside() override;                   // PC 0x00484860
        void DrawMouse(int) override;                    // stub, implements the base pure virtual
        void ChangeDriver(const char* psDriver) override; // PC 0x00484870
        const _GLC_RENDER_RESOLUTION* GetResolutions() override; // PC 0x00483040
        void ChangeResolution(const _GLC_RENDER_RESOLUTION* pResolution) override; // PC 0x004849B0

        // ZRenderWintel (new virtuals, PC vtable slots 116-155)
        virtual void Create(const char* pClassName, const char* pWindowName, uint32_t lStyle, uint32_t lExStyle, SZVRECT* pRect, void* pParent, int lUnknown); // PC 0x00484260
        virtual bool ShowWindow(int nCmdShow); // PC 0x004843C0
        virtual int WindowProc(uint32_t lMsg, uint32_t wParam, uint32_t lParam); // PC 0x00483B80
        virtual void OnCreate(uint32_t lParam); // PC 0x00484430
        virtual void OnDestroy(); // PC 0x00484450
        virtual void OnKeyDown(uint32_t wParam, uint32_t lParam); // PC slot 121 (stub)
        virtual void OnKeyUp(uint32_t wParam, uint32_t lParam); // PC slot 122 (stub)
        virtual void OnLButtonDown(int wParam, int lX, int lY); // PC 0x004838D0
        virtual void OnLButtonUp(int wParam, int lX, int lY); // PC 0x00486630
        virtual void OnLButtonDblClk(int wParam, int lX, int lY); // PC 0x004838E0
        virtual void OnRButtonDown(int wParam, int lX, int lY); // PC 0x00486680
        virtual void OnRButtonUp(int wParam, int lX, int lY); // PC 0x00486690
        virtual void OnMButtonDown(int wParam, int lX, int lY); // PC 0x004838C0
        virtual void OnMButtonUp(int wParam, int lX, int lY); // PC 0x004865C0
        virtual void OnMouseMove(int wParam, int lX, int lY); // PC 0x004838F0
        virtual int OnMouseWheel(short lWheelDelta); // PC 0x004866A0
        virtual bool OnSetCursor(); // PC 0x00483B20
        virtual void OnSetFocus(); // PC 0x00483A60
        virtual void OnKillFocus(); // PC 0x00483AD0
        virtual void OnPaint(int lPaintOnActivate) = 0; // pure (PC _purecall, slot 135)
        virtual void OnSize(unsigned int lWidth, unsigned int lHeight) = 0; // pure (PC _purecall, slot 136)
        virtual void Clear(); // PC slot 137 (empty; ZRenderWintelD3D overrides)
        virtual void ClipStripAndGetTriangles(REFTAB* pRefs, float* pPlanes, int lNumPlanes, float* pVertices, int lNumVertices, bool bForceWire); // PC 0x00483060
        virtual void ClipAndGetTriangles(REFTAB* pRefTab, float* pClipPlanes, int lNumVertices, float* pVertices, float* pPlaneNormals, bool bUnknown); // PC 0x004833B0
        virtual void DrawPrimitive(_GLC_PRIMITIVETYPE eType, const SVertexWintel* pVertices, uint32_t lNumVertices, const uint32_t* pIndices, uint32_t lNumIndices); // PC slot 140 (stub; iOS 0x1002412E4)
        virtual void DrawArrayBoxes(ZBaseGeom* pGeomBase, const ZVector3* pPos, void* pArrayData); // PC 0x00484AD0
        virtual void FlushSpriteList(float* pVertices, uint32_t lNumVertices, float (*pColors)[3], uint32_t lNumColors); // PC slot 142 (stub; iOS 0x1002412E8)
        virtual void Init() = 0; // pure (PC _purecall, slot 143)
        virtual void CreateRGBPalette(HDC hdc); // PC 0x00484540
        virtual void GetUserRect(SZVRECT* pRect); // PC 0x00484470
        virtual uint32_t Slot146(); // PC slot 146 (returns 0; TODO: name)
        virtual void MakeBackupBuffer(); // PC slot 147 (stub; iOS 0x1002412F4)
        virtual void FreeBackupBuffer(); // PC slot 148 (stub; iOS 0x1002412F8)
        virtual void RestoreBackupBuffer(); // PC slot 149 (stub; iOS 0x1002412FC)
        virtual void ResetRendererState(); // PC slot 150 (stub; iOS 0x100241300)
        virtual void SelectedBoxDump(const char* pText); // PC slot 151 (stub; iOS 0x10023F2E4)
        virtual bool BackupBufferExists(); // PC slot 152 (base returns false; ZRenderWintelD3D overrides with PC 0x004884F0)
        virtual int AdjustWindow(const char* pWindowTitle); // PC 0x00484590
        virtual void Slot154(uint32_t wParam); // PC slot 154 (stub, custom window-message handler 2031; TODO: name)
        virtual void UpdateBoxDump(const char* pText); // PC 0x00484240

        // methods
        ZRenderWintel(int hInstance, void* hParentWnd); // PC 0x00482EF0

        // members
        bool m_bClipCursor;              // +0x1354 (SetKeepInside / GetKeepInside)
        RE_ADD_PADDING(3);
        int m_lPaintOnActivate;          // +0x1358 (passed to OnPaint by WindowProc WM_PAINT / msg 2019)
        HCURSOR m_pCursor;               // +0x135C (GetCursor() captured at construction)
        int m_field1360;                 // +0x1360 (TODO: usage not yet mapped)
        int m_field1364;                 // +0x1364 (TODO: usage not yet mapped)
        int m_field1368;                 // +0x1368 (TODO: usage not yet mapped)
        int m_field136C;                 // +0x136C (TODO: usage not yet mapped)
        float m_fSpriteScale;            // +0x1370 (= 1.0; multiplies the sprite-array count in DrawArrayBoxes)
        int m_field1374;                 // +0x1374 (TODO: usage not yet mapped)
        char field_1378;                 // +0x1378
        bool m_bDisableSSE;              // +0x1379 (set from CPUID in ZRenderWintelD3D::Init)
        char field_137A;                 // +0x137A
        char field_137B;                 // +0x137B
        float m_fPixelAspect;            // +0x137C (= 1.0; overwritten by the "PixelAspect" option)
        _GLC_RENDER_RESOLUTION m_Resolutions[20]; // +0x1380 (20 x 20 bytes = 0x190; returned by GetResolutions)
        int m_field1510;                 // +0x1510 (TODO: usage not yet mapped)
        int m_field1514;                 // +0x1514 (TODO: usage not yet mapped)
        int m_field1518;                 // +0x1518 (TODO: usage not yet mapped)
        int m_field151C;                 // +0x151C (TODO: usage not yet mapped)
        char m_field1520;                // +0x1520 (zeroed by OnSetFocus)
        char field_1521;                 // +0x1521 (zeroed by OnSetFocus)
        char field_1522;                 // +0x1522
        char field_1523;                 // +0x1523
    };
    RE_VERIFY_SIZE(ZRenderWintel, 0x1524); // ZRenderX86 (0x1354) + Wintel members; verified by ZRenderWintel::Ctor (PC 0x00482EF0)
    RE_VERIFY_OFFSET(ZRenderWintel, m_bClipCursor, 0x1354);       // Verified by ZRenderWintel::Ctor / Dtor
    RE_VERIFY_OFFSET(ZRenderWintel, m_lPaintOnActivate, 0x1358);  // Passed to OnPaint (WindowProc)
    RE_VERIFY_OFFSET(ZRenderWintel, m_pCursor, 0x135C);           // GetCursor() write
    RE_VERIFY_OFFSET(ZRenderWintel, m_fSpriteScale, 0x1370);      // = 1.0f; scales the sprite count in DrawArrayBoxes
    RE_VERIFY_OFFSET(ZRenderWintel, m_fPixelAspect, 0x137C);      // = 1.0f
    RE_VERIFY_OFFSET(ZRenderWintel, m_Resolutions, 0x1380);       // GetResolutions returns &m_Resolutions
    RE_VERIFY_OFFSET(ZRenderWintel, m_field1520, 0x1520);         // Verified by ZRenderWintel::Ctor
}
