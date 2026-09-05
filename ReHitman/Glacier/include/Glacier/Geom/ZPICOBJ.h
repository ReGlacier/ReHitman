#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>


namespace Glacier
{
    class ZPICOBJ : public ZSTDOBJ
    {
    public:
        // vtbl
        virtual int ReadPixel(float const*,int *);
        virtual int ReadGlobalPixel(float const*,int *);
        virtual void SetDrawScale(float const*);
        virtual uint32_t InterpolateColor32(uint32_t,uint32_t,float);
        virtual int GetSysMemPixInterpolated(float const*,int,int *);
        virtual void ReadIndex4(uint8_t *,int,int,int,uint8_t *,uint8_t *,uint8_t *,uint8_t *);

        // data
        Glacier::ZVector3 m_vScale;
        Glacier::ZVector2 m_fOverrideSize;
        bool m_bKeepInRam;
        bool m_bCreateTexture;
        bool m_bNibbleEnabled;
        RE_ADD_PADDING(1);
        int m_lNrColors;
        REFTAB* m_prtMemPic;
    }; // Verified size 0x30
    RE_VERIFY_SIZE(ZPICOBJ, 0x30);
}