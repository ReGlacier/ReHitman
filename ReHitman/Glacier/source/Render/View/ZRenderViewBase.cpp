#include <Glacier/Render/PostFilter/ZPostFilter.h>
#include <Glacier/Render/View/ZRenderViewBase.h>
#include <Glacier/Render/Draw/ZDrawSurface.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZOldDrawInfo.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZCAMERA.h>
#include <Glacier/ZGameData.h> // WTF...
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZRenderViewBase::ZRenderViewBase(ZRender *pRender, IDraw *pDraw, unsigned int lViewNumber, unsigned int lViewId)
    {
        std::ignore = pDraw; // unused

        m_pRender = pRender;
        m_bPostfilterEnabled = false;
        m_pPostFilter = nullptr;
        m_pDrawSurface = nullptr;

        ZASSERT(lViewNumber < MAX_VIEWS_NR);
        m_lViewNumber = lViewNumber;
        m_lViewId = lViewId;

        m_lViewNumber = lViewNumber;
        m_lViewId = lViewId;
        m_pRender = pRender;
        m_lScissorOverride[0] = 0;
        m_lScissorOverride[1] = 0;
        m_lScissorOverride[2] = 0;
        m_lScissorOverride[3] = 0;
        m_lViewPort[0] = 0;
        m_lViewPort[1] = 0;
        m_lViewPort[2] = pRender->GetSizeX();
        m_lViewPort[3] = pRender->GetSizeY();
    }

    ZRender* ZRenderViewBase::Render()
    {
        return m_pRender;
    }

    void ZRenderViewBase::Init()
    {
        // Do nothing
    }
    
    void ZRenderViewBase::End()
    {
        // Do nothing
    }

    const uint32_t* ZRenderViewBase::Viewport()
    {
        return &m_lViewPort[0];
    }

    void ZRenderViewBase::EnablePostfilter()
    {
        const bool bBloodMoneyHack = (g_pGameData && g_pGameData->IsFrontEnd()); // For other games always false

        if (bBloodMoneyHack || g_pRenderDll->m_fPostFilterLOD != 0.0f)
        {
            m_bPostfilterEnabled = true;
            CreatePostFilter();
        }
        else
        {
            m_bPostfilterEnabled = false;
        }
    }

    void ZRenderViewBase::RemoveCameras()
    {
        if (m_Cameras.Count())
        {
            do
            {
                RemoveCamera(*m_Cameras.Get(0));
            }
            while (m_Cameras.Count());
        }

        IDraw::Instance()->Flush();
    }

    void ZRenderViewBase::SetViewport(const uint32_t* pViewport)
    {
        ZASSERT(pViewport);

        // Calc resolution
        const uint32_t lResW = m_pRender->GetSizeX();
        const uint32_t lResH = m_pRender->GetSizeY();

        const uint32_t x1 = pViewport[0];
        const uint32_t y1 = pViewport[1];
        const uint32_t x2 = pViewport[2];
        const uint32_t y2 = pViewport[3];

        // Validate
        ZASSERT(x1 < lResW);
        ZASSERT(y1 < lResH);
        ZASSERT(x2 > x1 && x2 <= lResW);
        ZASSERT(y2 > y1 && y2 <= lResH);

        // Store
        m_lViewPort[0] = x1;
        m_lViewPort[1] = y1;
        m_lViewPort[2] = x2;
        m_lViewPort[3] = y2;

        // Compute res
        const uint32_t width  = x2 - x1;
        const uint32_t height = y2 - y1;

        // Update PF
        if (m_pPostFilter)
        {
            m_pPostFilter->SetViewport(x1, y1, width, height);
        }

        // Update surface
        if (m_pDrawSurface)
        {
            m_pDrawSurface->SetViewport(x1, y1, width, height);
        }
    }
    
    void ZRenderViewBase::SetOverrideScissor(const uint32_t* pScissors)
    {
        m_lScissorOverride[0] = pScissors[0];
        m_lScissorOverride[1] = pScissors[1];
        m_lScissorOverride[2] = pScissors[2];
        m_lScissorOverride[3] = pScissors[3];
    }

    void ZRenderViewBase::AddCamera(ZCAMERA* pCamera)
    {
        ZASSERT(pCamera->IsDerivedFrom<ZCAMERA>());
        m_Cameras.Add(&pCamera, pCamera->CameraListPri); // ???
    }

    void ZRenderViewBase::RemoveCamera(ZCAMERA* pCamera)
    {
        IDraw::Instance()->Invalidate();
        ZASSERT(m_Cameras.Count()); // Need to have at least 1 camera to process
        
        if (const auto lIndex = m_Cameras.Find(&pCamera); lIndex >= 0)
        {
            m_Cameras.Remove(lIndex);
        }
    }

    ZCAMERA* ZRenderViewBase::GetCamera(int lIndex)
    {
        if (lIndex < 0 || lIndex >= m_Cameras.Count())
        {
            return nullptr;
        }

        return *m_Cameras.Get(lIndex);
    }

    void ZRenderViewBase::CreateSurface(ZDrawSurface::TARGET eTarget)
    {
        ZASSERT(m_pDrawSurface);

        if (eTarget == ZDrawSurface::TARGET::TEXTURE)
        {
            m_bOwnerDraw = true;
        }

        auto* pSurface = ZUniMemory::New<ZDrawSurface>(eTarget);
        if (pSurface)
        {
            m_pDrawSurface = pSurface;
        }
    }

    void ZRenderViewBase::GetDrawInfo(ZOldDrawInfo* pDrawInfo)
    {
        ZASSERT(pDrawInfo);

        // Accumulate data
        pDrawInfo->m_lNrTriangles += m_alNumTriangles[0];
        pDrawInfo->m_lNrSprites   += m_alNumTriangles[3];
        pDrawInfo->m_lNrSubList   += m_alNumPrimitives[0];
        pDrawInfo->m_lNrBonePrims += m_alNumDrawCalls[0];

        // Reset counters
        memset(m_alNumTriangles, 0, sizeof(m_alNumTriangles));
        memset(m_alNumPrimitives, 0, sizeof(m_alNumPrimitives));
        memset(m_alNumDrawCalls, 0, sizeof(m_alNumDrawCalls));

        // Collect camera
        if (m_Cameras.Count())
        {
            auto* pCamera = (*m_Cameras.Get(0));
            ZVector3 vCameraRootPos {};
            pCamera->GetRootPoint(vCameraRootPos);

            if (vCameraRootPos.x != 0.0f || vCameraRootPos.y != 0.0f || vCameraRootPos.z != 0.0f)
            {
                pDrawInfo->m_vCameraPosition = vCameraRootPos;
            }
        }
    }
    
    bool ZRenderViewBase::IsCameraInView(ZBaseGeom* pBaseGeom)
    {
        if (!m_Cameras.Count())
        {
            return false;
        }

        for (int i = 0; i < m_Cameras.Count(); ++i)
        {
            auto* pCamera = *m_Cameras.Get(i);

            if (pCamera && pCamera->BaseGeom() == pBaseGeom)
            {
                return true;
            }
        }

        return false;
    }

    void ZRenderViewBase::FreeDeviceBuffers()
    {
        if (m_pPostFilter)
        {
            m_pPostFilter->FreeDeviceBuffers();
        }
    }

    void ZRenderViewBase::AllocateDeviceBuffers()
    {
        if (m_pPostFilter)
        {
            m_pPostFilter->AllocateDeviceBuffers();
        }
    }
    
    void ZRenderViewBase::CreatePostFilter()
    {
        // Do nothing
    }
    
    ZPostFilter* ZRenderViewBase::GetPostFilter()
    {
        return m_pPostFilter;
    }
    
    ZPostFilter* ZRenderViewBase::FindPostFilterForCamera(ZBaseGeom* pCameraGeom)
    {
        return IsCameraInView(pCameraGeom) ? GetPostFilter() : nullptr;
    }
}