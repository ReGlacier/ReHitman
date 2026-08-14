#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class REFTAB;
    struct SPrims;

    class ZPrimControlBase
    {
    public:
        // vtbl (not finished)
        virtual void GetPrimExtraData(uint32_t, uint32_t, void*, uint32_t);
        virtual void GetPrimTriangles(REFTAB* ppTris, SPrims* pPrims, uint32_t lPrim) = 0;
        virtual void GetPrimTriangles(REFTAB* ppTris, uint32_t lPrim) = 0;
        virtual int ShouldUseBoundForLightCheck(uint32_t lPrim);
        virtual uint8_t GetPrimPackType(uint32_t lPrim);
        virtual const void* GetPrimData(uint32_t lPrim);
        virtual uint32_t GetPrimOffset(const void* ptr);
        virtual bool CheckInPrimBuffer(const void* ptr);
        virtual bool IsPrimUnique(uint32_t lPrim);
        virtual bool CheckPointInsidePrim(uint32_t lPrim, const ZVector3& vPoint, float fThreshold);

        // tbd

        // methods
        static ZPrimControlBase* Instance();

        // members
    };
}