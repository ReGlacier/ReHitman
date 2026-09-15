#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZOpeningBase
    {
    public:
        // vftable
        virtual void* FindReferenceNormal();
        virtual void FindRootNormal();
        virtual void FindGates();
        virtual ZGEOM* GetBaseGeom();
        virtual void AdjustOpenness(float open, float close);
        virtual void Release(bool);
        virtual void Init();
        virtual void Init2();

        // members
        uint16_t m_ExitIndices;
        uint16_t m_pad6;
        bool m_bNotifyGate;
        char m_nReferenceAxis;
        ZVector3 m_vRootNormal;
    };
    RE_VERIFY_SIZE(ZOpeningBase, 0x);
}