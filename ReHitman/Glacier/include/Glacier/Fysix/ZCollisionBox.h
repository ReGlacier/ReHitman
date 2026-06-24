#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
	struct ZCollisionBox
	{
        uint32_t m_rContactGeom;
        uint32_t m_lNrStrips;
        char* m_pMemBuffer;
        uint32_t m_lMemBufferSize;
        ZMat3x3 m_mBoxMatrix;
        ZVector3 m_vBoxPosition;
        ZVector3 m_vBoxDimensions;
        TIMETYPE m_fFrameTime;
        float m_fDeltaFrameTime;
        ZMat3x3 m_mPushBoxMatrix;
        ZVector3 m_vPushBoxPosition;
        ZVector3 m_vPushBoxDimensions;
        bool m_bPushBoxEnabled;
        bool m_bLocked;
	};
    RE_VERIFY_SIZE(ZCollisionBox, 0x94); // From PS2, unconfirmed
}