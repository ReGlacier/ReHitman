#include <Glacier/Render/Draw/ZRenderDrawBase.h>
#include <Glacier/Render/View/ZRenderViewBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderDrawBase::ZRenderDrawBase()
    {
        m_bValid = false;
        m_bInitialized = false;
        memset(m_ViewNumberToPointer, 0, sizeof(m_ViewNumberToPointer));
    }

    bool ZRenderDrawBase::IsValid()
    {
        return m_bValid;
    }

    bool ZRenderDrawBase::ShouldFlush()
    {
        return !m_bValid;
    }

    void ZRenderDrawBase::InitAllocation()
    {
        // Do nothing
    }

    int ZRenderDrawBase::GetLargestFreeBlock()
    {
        return 0;
    }

    void* ZRenderDrawBase::Alloc(int lSize, const char* pFile, uint32_t lLine)
    {
        return ZUniMemory::Allocate(lSize);
    }

    void ZRenderDrawBase::Free(void* pMem, int)
    {
        ZUniMemory::Free(pMem);
    }

    void* ZRenderDrawBase::AllocSprites(int lSize)
    {
        return ZUniMemory::Allocate(lSize);
    }

    void ZRenderDrawBase::FreeSprites(void* pMem, int)
    {
        ZUniMemory::Free(pMem);
    }

    IView* ZRenderDrawBase::CreateView(ZRender* pRender, uint32_t lViewId, ZDrawSurface::TARGET lTarget)
    {
        Invalidate();

        uint32_t lFoundIndex = MAX_VIEWS_NR + 1;
        for (int i = 0; i < MAX_VIEWS_NR; ++i)
        {
            if (!m_ViewNumberToPointer[i])
            {
                lFoundIndex = i;
                break;
            }
        }

        ZASSERT(lFoundIndex < MAX_VIEWS_NR);

        auto* pNewView = NewView(pRender, lFoundIndex, lViewId);
        pNewView->CreateSurface(lTarget);

        if (m_bInitialized)
        {
            pNewView->Init();
        }

        ZASSERT(!m_ViewNumberToPointer[pNewView->m_lViewNumber]);
        m_ViewNumberToPointer[pNewView->m_lViewNumber] = pNewView;

        m_Views.Add(&pNewView, 1u - static_cast<uint8_t>(lTarget));
        return pNewView;
    }

    IView* ZRenderDrawBase::FindView(uint32_t lViewId)
    {
        if (m_Views.IsEmpty())
        {
            return nullptr;
        }

        for (int i = 0; i < MAX_VIEWS_NR; ++i)
        {
            auto** ppView = m_Views.Get(i);
            auto* pView = *ppView;

            if (pView && pView->m_lViewId == lViewId)
            {
                return pView;
            }
        }

        return nullptr;
    }

    IView* ZRenderDrawBase::GetViewByIndex(int32_t lViewIndex)
    {
        return lViewIndex < m_Views.Count() ? *m_Views.Get(lViewIndex) : nullptr;
    }

    bool ZRenderDrawBase::RemoveView(IView* pView)
    {
        Flush();

        if (m_Views.IsEmpty())
        {
            return false;
        }

        auto* pCastedView = reinterpret_cast<ZRenderViewBase*>(pView);
        const auto lFoundIndex = m_Views.Find(&pCastedView);
        ZASSERT((lFoundIndex >= 0 && lFoundIndex < m_Views.Count()) && "View is not contains in m_Views");
        if (lFoundIndex < 0)
            return false;

        m_Views.Remove(lFoundIndex);

        // Reset view
        pView->RemoveCameras();
        pView->End();

        m_ViewNumberToPointer[pCastedView->m_lViewNumber] = 0;
        ZUniMemory::Delete(pView);

        Invalidate();
        return true;
    }

    void ZRenderDrawBase::RemoveViewsUsingRender(ZRender* pRender)
    {
        for (uint32_t i = 0; i < m_Views.Count(); )
        {
            auto** ppView = m_Views.Get(i);
            auto* pView = *ppView;
            ZASSERT(pView);

            if (pView->m_pRender == pRender)
            {
                RemoveView(pView);
                i = 0;
            }
            else
            {
                ++i;
            }
        }
    }

    void ZRenderDrawBase::Init()
    {
        for (int i = 0; i < m_Views.Count(); ++i)
        {
            auto** ppView = m_Views.Get(i);
            auto* pView = *ppView;

            pView->Init();
        }

        m_bInitialized = true;
    }

    void ZRenderDrawBase::End()
    {
        if (!m_bInitialized)
        {
            return;
        }

        for (int i = 0; i < m_Views.Count(); ++i)
        {
            auto** ppView = m_Views.Get(i);
            auto* pView = *ppView;

            pView->End();
            ZUniMemory::Delete(pView);
        }

        m_Views.Clear();
        m_bInitialized = false;
    }
    
    void ZRenderDrawBase::Flush()
    {
        m_bValid = true;
    }

    void ZRenderDrawBase::Update(ZRender* pRender)
    {
        // Do nothing
    }

    void ZRenderDrawBase::BeginFrame()
    {
        // Do nothing
    }

    void ZRenderDrawBase::EndFrame()
    {
        // Do nothing
    }

    void ZRenderDrawBase::Invalidate()
    {
        m_bValid = false;
    }

    void ZRenderDrawBase::AddBoneMark(ZBaseGeom* AddBoneMark, const float* vPosition, const float* vDirection, float fRadius, uint32_t lBoneId, uint32_t lSourcePrim)
    {
        // Do nothing
    }

    void ZRenderDrawBase::UpdateBaseGeom(ZBaseGeom* pBaseGeom)
    {
        // Do nothing   
    }

    void ZRenderDrawBase::ChangePrim(ZBaseGeom* pBaseGeom, uint32_t lPrim)
    {
        // Do nothing
    }

    bool ZRenderDrawBase::ValidateCaster(const ZBaseGeom* pBaseGeom)
    {
        // Do nothing
        return true;
    }

    const ZBone* ZRenderDrawBase::GetBaseGeomBones(const ZBaseGeom* pBaseGeom)
    {
        // Do nothing
        return nullptr;
    }

    bool ZRenderDrawBase::ValidateReceiver(const ZBaseGeom* pBaseGeom)
    {
        // Do nothing
        return true;
    }

    void ZRenderDrawBase::EnableOwnerDraw(const ZBaseGeom* pBaseGeom)
    {
        // Do nothing
    }

    void ZRenderDrawBase::DisableOwnerDraw(const ZBaseGeom* pBaseGeom)
    {
        // Do nothing
    }

    void ZRenderDrawBase::GetDrawInfo(ZOldDrawInfo* pDrawInfo)
    {
        for (int i = 0; i < m_Views.Count(); ++i)
        {
            auto** ppView = m_Views.Get(i);
            auto* pView = *ppView;

            pView->GetDrawInfo(pDrawInfo);
        }
    }

    void ZRenderDrawBase::CreateDefaultBones(ZBone* pBone, ZLNKOBJ* pLnkObj)
    {
        // Do nothing
    }

    ZPostFilter* ZRenderDrawBase::FindPostFilterForCamera(ZBaseGeom* pCamera)
    {
        if (m_Views.IsEmpty())
        {
            return nullptr;
        }

        for (int i = 0; i < m_Views.Count(); ++i)
        {
            auto** ppView = m_Views.Get(i);
            auto* pView = *ppView;

            auto* pFound = pView->FindPostFilterForCamera(pCamera);
            if (pFound)
            {
                return pFound;
            }
        }

        // Not found
        return nullptr;
    }

    void ZRenderDrawBase::UpdateRenderSize(ZRender* pRender, uint32_t lWidth, uint32_t lHeight)
    {
        uint32_t aVP[4] { 0, 0, lWidth, lHeight };
        const float fHorizontalScaleFactor = g_pSysInterface->m_lResolutionWanted[0] == -1 ? 1.f : static_cast<float>(lWidth) / static_cast<float>(g_pSysInterface->m_lResolutionWanted[0]);
        const float fVerticalScaleFactor = g_pSysInterface->m_lResolutionWanted[1] == -1 ? 1.f : static_cast<float>(lHeight) / static_cast<float>(g_pSysInterface->m_lResolutionWanted[1]);

        for (int i = 0; i < m_Views.Count(); ++i)
        {
            auto** ppView = m_Views.Get(i);
            auto* pView = *ppView;

            if (pView->m_pRender != pRender)
            {
                continue;
            }

            if (g_pSysInterface->m_bFullScreen)
            {
                const auto* pViewport = pView->Viewport();
                aVP[0] = pViewport[0] * fHorizontalScaleFactor;
                aVP[1] = pViewport[1] * fVerticalScaleFactor;
                aVP[2] = pViewport[2] * fHorizontalScaleFactor;
                aVP[3] = pViewport[3] * fVerticalScaleFactor;
            }

            pView->SetViewport(aVP);
        }
    }

    void ZRenderDrawBase::FreeDeviceBuffers()
    {
        // Do nothing
    }

    void ZRenderDrawBase::AllocateDeviceBuffers()
    {
        // Do nothing
    }
    
    ZStackArrayInsert<32, ZRenderViewBase*, uint8_t>* ZRenderDrawBase::GetViewList()
    {
        return &m_Views;
    }
}