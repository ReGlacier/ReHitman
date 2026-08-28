#include <Glacier/Render/ZRenderX86.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/ZRenderBaseDll.h>


namespace Glacier
{
    // PC 0x0047E5C0. X86-specific construction on top of ZRender: stores the instance handle,
    // zeroes the render/D3D state fields and initialises the wire-mode + mouse-move flags.
    ZRenderX86::ZRenderX86(int hInstance, void* hParentWnd)
        : ZRender(hInstance, hParentWnd)
    {
        m_ParenthWnd = hParentWnd;
        m_pContext = nullptr;
        m_hWnd = nullptr;
        m_lDrawBufferCon = 0;
        m_lDrawConForbidRender = 0;
        m_lDrawConForbidCamera = 0;
        m_bViewLocked = false;
        m_bForceWire = false;
        m_bWrapCursor = false;
        m_oldScrMouseX = 0;
        m_oldScrMouseY = 0;
        m_lNumPostEffects = 0;
        m_bCullingReversed = false;
        m_hInstance = hInstance;
        m_lWireMode = 1;
        m_MouseMoveUpdate = true;
    }

    // PC 0x0047E7F0. Sets the vtable, frees the draw base buffers, deletes the render
    // context, then lets the base ZRender destructor unlink / end the draw base.
    ZRenderX86::~ZRenderX86()
    {
        ZRender::FreeDrawBuffers();

        if (m_pContext)
        {
            operator delete(m_pContext);
            m_pContext = nullptr;
        }
    }

    // PC 0x0047E7B0
    bool ZRenderX86::BeginScene()
    {
        ZRender::BeginScene();

        IDraw* pDraw = IDraw::Instance();
        if (!pDraw->IsValid())
            IDraw::Instance()->Flush();

        if (g_pSysInterface->m_lShadowDetail)
            HandleShadows();

        return true;
    }

    // PC 0x0047E660 (forwarding thunk to the base implementation)
    void ZRenderX86::AllocateDrawBuffers()
    {
        ZRender::AllocateDrawBuffers();
    }

    // PC 0x0047E670 (forwarding thunk to the base implementation)
    void ZRenderX86::FreeDrawBuffers()
    {
        ZRender::FreeDrawBuffers();
    }

    // PC 0x0047E680. Skips the frame update while the render view is locked
    // (m_bViewLocked at +0x1348), otherwise runs the base ZRender::Update.
    void ZRenderX86::Update()
    {
        if (!m_bViewLocked)
            ZRender::Update();
    }

    // PC 0x00761544 (nullsub)
    void ZRenderX86::ChangeDriver(const char*)
    {
        // Do nothing
    }

    // PC 0x00761548 (nullsub)
    void ZRenderX86::ChangeDevice(const char*)
    {
        // Do nothing
    }

    // PC 0x0076154C (nullsub)
    void ZRenderX86::ChangeResolution(const _GLC_RENDER_RESOLUTION*)
    {
        // Do nothing
    }

    // PC 0x006B3640 (return-nullptr stub)
    const _GLC_RENDER_RESOLUTION* ZRenderX86::GetResolutions()
    {
        return nullptr;
    }

    // PC 0x00761550 (nullsub)
    void ZRenderX86::GetRenderCaps(_GLC_RENDERCAPS*)
    {
        // Do nothing
    }

    // PC 0x0047E740. Case 6 writes the gamma value (pValue dereferenced as float bits);
    // case 9 rebuilds the character LOD table, using the pValue bits directly as the LOD index.
    void ZRenderX86::SetRenderState(const _GLC_RENDERSTATE eRenderState, const unsigned int* pValue)
    {
        switch (eRenderState)
        {
        case GLC_RENDERSTATE_GAMMA:
            SetGammaValue(*reinterpret_cast<const float*>(pValue));
            break;
        case GLC_RENDERSTATE_LOD:
            SetLOD(m_LODTableCharacters, reinterpret_cast<uint32_t>(pValue), 6);
            break;
        default:
            break;
        }
    }

    // PC 0x0047E690
    void ZRenderX86::GetRenderState(const _GLC_RENDERSTATE eRenderState, unsigned int* pOutValue)
    {
        switch (eRenderState)
        {
        case GLC_RENDERSTATE_ANISOTROPY:
            *pOutValue = g_pRenderDll->m_lAnisotropy;
            break;
        case GLC_RENDERSTATE_ANTIALIAS:
            *pOutValue = g_pRenderDll->m_lAntialias;
            break;
        case GLC_RENDERSTATE_DXT:
            *pOutValue = !g_pRenderDll->m_bDisableDXT;
            break;
        case GLC_RENDERSTATE_EMBM:
            *pOutValue = !g_pRenderDll->m_bDisableEMBM;
            break;
        case GLC_RENDERSTATE_TRILINEAR:
            *pOutValue = g_pRenderDll->m_bTrilinearFiltering;
            break;
        case GLC_RENDERSTATE_LOD:
            *pOutValue = 100;
            break;
        default:
            break;
        }
    }

    // PC 0x006F7FC0 (shared return-false stub)
    bool ZRenderX86::EnableFog(bool)
    {
        return false;
    }

    // PC 0x0047E780
    void ZRenderX86::SetGammaValue(const float fGamma)
    {
        g_pRenderDll->m_fGammaValue = fGamma;
    }

    // PC 0x0047E7A0
    float ZRenderX86::GammaValue()
    {
        return g_pRenderDll->m_fGammaValue;
    }

    // PC 0x006B3640 (return-0 stub)
    unsigned int ZRenderX86::ShadowRenderLayerId()
    {
        return 0;
    }

    // PC 0x006F7FC0 (shared return-false stub)
    bool ZRenderX86::ShadowRenderStart(unsigned int)
    {
        return false;
    }

    // PC 0x005643F0 (return-0 stub)
    unsigned int ZRenderX86::NumShadowRenderCasterPasses(unsigned int)
    {
        return 0;
    }

    // PC 0x005BD050 (shared return-0 stub)
    unsigned int ZRenderX86::ShadowRenderCasterStart(unsigned int, unsigned int)
    {
        return 0;
    }

    // PC 0x00761550 (nullsub)
    void ZRenderX86::ShadowRenderCasters(unsigned int)
    {
        // Do nothing
    }

    // PC 0x00761558 (empty stub)
    void ZRenderX86::ShadowRenderReceivers()
    {
        // Do nothing
    }

    // PC 0x0076155C (empty stub)
    void ZRenderX86::ShadowRenderEnd()
    {
        // Do nothing
    }

    // PC 0x00761560 (nullsub)
    void ZRenderX86::SetScissorRect(float, float, float, float, bool)
    {
        // Do nothing
    }

    // PC 0x0047E650
    void ZRenderX86::SetCullingReversed(bool bCullingReversed)
    {
        m_bCullingReversed = bCullingReversed;
    }

    // PC 0x0069D5C0 (shared return-false stub)
    bool ZRenderX86::MirrorRenderBegin()
    {
        return false;
    }

    // PC 0x00761564 (empty stub)
    void ZRenderX86::MirrorRenderReflectors()
    {
        // Do nothing
    }

    // PC 0x00761568 (empty stub)
    void ZRenderX86::MirrorRenderReflected()
    {
        // Do nothing
    }

    // PC 0x0069D5C0 (shared return-false stub)
    bool ZRenderX86::MirrorRenderZHole()
    {
        return false;
    }

    // PC 0x0076156C (empty stub)
    void ZRenderX86::MirrorRenderEnd()
    {
        // Do nothing
    }

    // PC 0x00593F70 (return-nullptr stub)
    void* ZRenderX86::LockOffscreenSurface(unsigned int*, int*, int*, SZVRECT*)
    {
        return nullptr;
    }

    // PC 0x00761570 (nullsub)
    void ZRenderX86::UnlockOffscreenSurface(void*)
    {
        // Do nothing
    }

    // PC 0x00761574 (empty stub)
    void ZRenderX86::BlitOffscreenSurface()
    {
        // Do nothing
    }

    // PC 0x00448720 (shared return-false stub)
    bool ZRenderX86::CreateOffscreenSurface(int, int)
    {
        return false;
    }

    // PC 0x00761578 (empty stub)
    void ZRenderX86::DestroyOffscreenSurface()
    {
        // Do nothing
    }

    // PC 0x0076157C (nullsub)
    void ZRenderX86::UpdateCamera(ZCAMERA*)
    {
        // Do nothing
    }
}
