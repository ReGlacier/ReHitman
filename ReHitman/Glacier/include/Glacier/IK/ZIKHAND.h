#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZItem;
    class ZItemTemplate;
    class ZIKLNKOBJ;

    struct ZTARGET
    {
        // FIXME: Fix vtbl method
        virtual ~ZTARGET();

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
        void(*m_Callback[4])(ZLNKOBJ*);
    };
    RE_VERIFY_SIZE(ZTARGET, 0x60);
    RE_VERIFY_OFFSET(ZTARGET, m_bEnabled, 0x8); // See ZTARGET::ZTARGET for details (PC at 0x0050CA30)

    struct SHandInfo
    {
        uint32_t m_rItem;
        bool m_bIKItemEnabled;
        bool m_pad5[3];
    };
    RE_VERIFY_SIZE(SHandInfo, 0x8);

    class ZIKHAND : public ZTARGET
    {
    public:
        virtual void LoadSave(ZPackedInput*, bool);
        virtual void AttachItem(ZIKLNKOBJ* owner, Glacier::ZREF itemID);
        virtual void SlipItem(ZIKLNKOBJ* owner);
        virtual ZItem* GetItem();
        virtual ZItemTemplate* GetItemType();
        virtual void Reset();
        virtual void SetTarget(ZIKLNKOBJ* owner, const Matrix4x4* transform, const Vector3* point, float veliocity, void* callback);

        // data
        SHandInfo m_HandInfo;
        unsigned int m_lBoneId;
        Glacier::ZVector3 m_vItemOffset;
        uint16_t m_msgInventorySetActive;
        uint8_t m_pad2[6];
    };
    RE_VERIFY_SIZE(ZIKHAND, 0x80); // Verified for PC
}