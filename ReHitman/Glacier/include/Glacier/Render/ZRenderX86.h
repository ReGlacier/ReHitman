#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Common.h>


namespace Glacier
{
    class ZRenderContext;
    class ZCAMERA;

    class ZRenderX86 : public ZRender
    {
    public:
        // vtbl
        ~ZRenderX86() override;
        bool BeginScene() override;
        void AllocateDrawBuffers() override;
        void FreeDrawBuffers() override;
        void Update() override;

        // ZRenderX86 (new virtuals, PC vtable slots 86-115)
        virtual void ChangeDriver(const char* psDriver);
        virtual void ChangeDevice(const char* psDevice);
        virtual void ChangeResolution(const _GLC_RENDER_RESOLUTION* pResolution);
        virtual const _GLC_RENDER_RESOLUTION* GetResolutions();
        virtual void GetRenderCaps(_GLC_RENDERCAPS* pRenderCaps);
        virtual void SetRenderState(const _GLC_RENDERSTATE eRenderState, const unsigned int* pValue);
        virtual void GetRenderState(const _GLC_RENDERSTATE eRenderState, unsigned int* pOutValue);
        virtual bool EnableFog(bool bEnable);
        virtual void SetGammaValue(const float fGamma);
        virtual float GammaValue();
        virtual unsigned int ShadowRenderLayerId();
        virtual bool ShadowRenderStart(unsigned int lLayerId);
        virtual unsigned int NumShadowRenderCasterPasses(unsigned int lLayerId);
        virtual unsigned int ShadowRenderCasterStart(unsigned int lLayerId, unsigned int lCaster);
        virtual void ShadowRenderCasters(unsigned int lNum);
        virtual void ShadowRenderReceivers();
        virtual void ShadowRenderEnd();
        virtual void SetScissorRect(float l, float t, float r, float b, bool bEnable);
        virtual void SetCullingReversed(bool bCullingReversed);
        virtual bool MirrorRenderBegin();
        virtual void MirrorRenderReflectors();
        virtual void MirrorRenderReflected();
        virtual bool MirrorRenderZHole();
        virtual void MirrorRenderEnd();
        virtual void* LockOffscreenSurface(unsigned int* pWidth, int* pHeight, int* pPitch, SZVRECT* pRect);
        virtual void UnlockOffscreenSurface(void* pSurface);
        virtual void BlitOffscreenSurface();
        virtual bool CreateOffscreenSurface(int lWidth, int lHeight);
        virtual void DestroyOffscreenSurface();
        virtual void UpdateCamera(ZCAMERA* pCamera);

        // methods
        ZRenderX86(int hInstance, void* hParentWnd); // PC 0x0047E5C0

        // members
        int m_hInstance;                 // +0x31C
        void* m_hWnd;                    // +0x320
        void* m_ParenthWnd;              // +0x324
        uint8_t m_axCompiled[0x1000];    // +0x328
        int m_lDrawBufferCon;            // +0x1328
        uint32_t m_lDrawConForbidRender; // +0x132C
        uint32_t m_lDrawConForbidCamera; // +0x1330
        uint32_t m_lDrawMask;            // +0x1334
        uint32_t m_lWireMode;            // +0x1338
        bool m_bForceWire;               // +0x133C
        bool m_bCullingReversed;         // +0x133D
        bool m_MouseMoveUpdate;          // +0x133E
        bool m_bWrapCursor;              // +0x133F
        int m_oldScrMouseX;              // +0x1340
        int m_oldScrMouseY;              // +0x1344
        bool m_bViewLocked;              // +0x1348
        RE_ADD_PADDING(3);               // +0x1349
        ZRenderContext* m_pContext;      // +0x134C
        uint32_t m_lNumPostEffects;      // +0x1350
    };
    RE_VERIFY_SIZE(ZRenderX86, 0x1354); // Base ZRender (0x31C) + X86 members; verified by ZRenderX86::Ctor (PC 0x0047E5C0)
    RE_VERIFY_OFFSET(ZRenderX86, m_hWnd, 0x320);            // Verified by ZRenderX86::Ctor
    RE_VERIFY_OFFSET(ZRenderX86, m_axCompiled, 0x328);      // XBOX MiniNinjas / KL2 layout
    RE_VERIFY_OFFSET(ZRenderX86, m_lDrawBufferCon, 0x1328); // Verified by ZRenderX86::Ctor
    RE_VERIFY_OFFSET(ZRenderX86, m_lWireMode, 0x1338);      // Verified by ZRenderX86::Ctor (= 1)
    RE_VERIFY_OFFSET(ZRenderX86, m_MouseMoveUpdate, 0x133E);// Verified by ZRenderX86::Ctor (= 1)
    RE_VERIFY_OFFSET(ZRenderX86, m_bViewLocked, 0x1348);    // Checked by ZRenderX86::Update (PC 0x0047E680)
    RE_VERIFY_OFFSET(ZRenderX86, m_pContext, 0x134C);       // Verified by ZRenderX86::Ctor / Dtor
    RE_VERIFY_OFFSET(ZRenderX86, m_lNumPostEffects, 0x1350);// Verified by ZRenderX86::Ctor
}
