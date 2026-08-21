#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/IK.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZLNKOBJ;

    class ZTARGET
    {
    public:
        // vtbl
        virtual void LoadSave(ZLNKOBJ* pLnkObj, ISerializerStream& stream, bool bSaving);

        // methods
        ZTARGET();

        void SetBoneId(int lBoneId);
        float GetTimePrc() const;
        void SetTime(float fTime, bool bRemove);
        bool GetPos(ZVector3& vPos) const;
        void CallBackIfTime(ZIKLNKOBJ* pIkLnkObj);
        void CallBackAndRemove(ZIKLNKOBJ* pDest);
        void Clear();
        void Enable(bool bEnabled);
        bool IsEnabled() const;
        bool Removing() const;
        void SetMatPosGeom(const ZMat3x3* pMat, const ZVector3* pPos, ZREF rGeom);
        void SetCallBack(ZIKCALLBACK cb);

        // members
        uint8_t m_pad0[4]; // PADDING
        bool m_bEnabled;
        uint8_t m_pad1[3]; // PADDING
        ZMat3x3 m_mTarget;
        ZVector3 m_vTarget;
        float m_fTime;
        float m_fStartTime;
        uint32_t m_rGeom;
        int m_lBoneId;
        ZLNKOBJ* m_pLnkObj;
        ZIKCALLBACK m_CallBack;
    };
    RE_VERIFY_SIZE(ZTARGET, 0x60);
    RE_VERIFY_OFFSET(ZTARGET, m_bEnabled, 0x8); // See ZTARGET::ZTARGET for details (PC at 0x0050CA30)
}
