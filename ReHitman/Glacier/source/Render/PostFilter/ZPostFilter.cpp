#include <Glacier/Render/PostFilter/ZPostFilter.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZPostFilter::ZPostFilter()
    {
        m_bFirstPersonOnlyInBuffer1 = false;
        m_bMenuEnabled = false;
        m_bMotionBlur = false;
        m_fDepthBlur = 0.0f;
        m_fLastBlur = 0.0f;
        m_fLastBlendAmountLastBuffer = 0.0f;
        m_fLastBlendAmountFrameBuffer = 1.0f;
        m_fLastBlendAngleX = 0.0f;
        m_fLastBlendAngleY = 0.0f;
        m_fLastBlendZoomX = 1.0f;
        m_fLastBlendZoomY = 1.0f;
        m_fFinishBlendAmount = 0.0f;
        m_fFinishSelectAmount = 0.0f;
        m_fFinishBlur = 0.0f;
        m_bCurveMenuOverride = false;
        m_fNoiseScale = 1.0f;
        m_fNoiseMin = 0.0f;
        m_fNoiseMax = 0.0f;
        m_fNoiseColor[0] = 0.0f;
        m_fNoiseColor[1] = 0.0f;
        m_fNoiseColor[2] = 0.0f;
        m_fBloomAmount = 0.0f;
        m_fBloomBlurriness = 0.0f;
        m_fHeatShimmerSpeed = 0.0f;
        m_fHeatShimmerUScale = 0.0f;
        m_fHeatShimmerVScale = 0.0f;
    }

    ZPostFilter::~ZPostFilter() = default;
    
    void ZPostFilter::Flush()
    {
        // Do nothing
    }

    void ZPostFilter::Update(ZRenderViewBase* pView)
    {
        // Do nothing
    }
    
    void ZPostFilter::Init()
    {
        // Do nothing
    }

    void ZPostFilter::FrameUpdate()
    {
        // Do nothing
    }

    void ZPostFilter::UpdateDynamic()
    {
        // Do nothing
    }

    void ZPostFilter::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
    {
        // Do nothing
    }

    void ZPostFilter::FreeDeviceBuffers()
    {
        // Do nothing
    }

    void ZPostFilter::AllocateDeviceBuffers()
    {
        // Do nothing
    }

    int ZPostFilter::UnlockRedPalette(uint32_t lIndex)
    {
        return 1;
    }

    int ZPostFilter::UnlockGreenPalette(uint32_t lIndex)
    {
        return 1;
    }

    int ZPostFilter::UnlockBluePalette(uint32_t lIndex)
    {
        return 1;
    }
    
    int ZPostFilter::UnlockZBufferPalette()
    {
        return 1;
    }

    int ZPostFilter::IsZPassEnable()
    {
        return 0;
    }
    
    void ZPostFilter::EnableZPass(int lEnable)
    {
        // Do nothing
    }

    void ZPostFilter::SetDepthBlurriness(float fBlurriness)
    {
        if (fBlurriness >= 0.00012207031f)
        {
            m_fDepthBlur = fBlurriness;
        }
        else
        {
            m_fDepthBlur = 0.f;
        }
    }

    void ZPostFilter::SetHeatShimmerSpeed(float fSpeed)
    {
        if (fSpeed >= 0.00012207031f)
        {
            m_fHeatShimmerSpeed = fSpeed;
        }
        else
        {
            m_fHeatShimmerSpeed = 0.f;
        }
    }

    bool ZPostFilter::GotMotionBlur()
    {
        return false;
    }

    void ZPostFilter::SaveScreenShot()
    {
        ZASSERT(false);
    }
}