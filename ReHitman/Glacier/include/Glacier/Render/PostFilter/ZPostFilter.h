#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    class ZPostFilter
    {
    public:
        // vtbl
        virtual ~ZPostFilter();
        virtual void Flush();
        virtual void Update(ZRenderViewBase* pView);
        virtual void Init();
        virtual void FrameUpdate();
        virtual void UpdateDynamic();
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        virtual void FreeDeviceBuffers();
        virtual void AllocateDeviceBuffers();
        virtual uint32_t* GetRedPalette(uint32_t lIndex) = 0;
        virtual uint32_t* GetGreenPalette(uint32_t lIndex) = 0;
        virtual uint32_t* GetBluePalette(uint32_t lIndex) = 0;
        virtual uint8_t* GetZBufferPalette() = 0;
        virtual int UnlockRedPalette(uint32_t lIndex);
        virtual int UnlockGreenPalette(uint32_t lIndex);
        virtual int UnlockBluePalette(uint32_t lIndex);
        virtual int UnlockZBufferPalette();
        virtual int IsZPassEnable();
        virtual void EnableZPass(int lEnable);
        virtual void SetDepthBlurriness(float fBlurriness);
        virtual void SetHeatShimmerSpeed(float fSpeed);
        virtual bool GotMotionBlur();
        virtual void UpdateCurvePalettes() = 0;
        virtual void SaveScreenShot();

        // methods
        ZPostFilter();

        // members
        bool m_bFirstPersonOnlyInBuffer1; // +0x4
        bool m_bMenuEnabled; // +0x8
        struct ZCurvesMenu* m_pCurvesMenu; // Debug stuff
        bool m_bCurveMenuOverride;
        float m_fDepthBlur; // Verified 
        float m_fLastBlur;
        float m_fLastBlendAmountLastBuffer;
        float m_fLastBlendAmountFrameBuffer;
        float m_fLastBlendAngleX;
        float m_fLastBlendAngleY;
        float m_fLastBlendZoomX;
        float m_fLastBlendZoomY;
        float m_fFinishBlendAmount;
        float m_fFinishSelectAmount;
        float m_fFinishBlur;
        float m_fOffset[3][2];
        bool m_bMotionBlur;
        float m_fNoiseScale;
        float m_fNoiseMin;
        float m_fNoiseMax;
        float m_fNoiseColor[3];
        float m_fBloomAmount;
        float m_fBloomBlurriness;
        float m_fHeatShimmerSpeed;
        float m_fHeatShimmerUScale;
        float m_fHeatShimmerVScale;
        ZMatrix m_mViewMatrix;
        char *m_pPostfilterName;
    };

    RE_VERIFY_OFFSET(ZPostFilter, m_fDepthBlur, 0x10); // Verified by ZPostFilter::SetDepthBlurriness
    RE_VERIFY_OFFSET(ZPostFilter, m_fHeatShimmerSpeed, 0x78); // Verified by ZPostFilter::SetHeatShimmerSpeed
}