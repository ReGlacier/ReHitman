#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/IK/ZTARGET.h>


namespace Glacier
{
    // fwds
    class ZItem;
    class ZItemTemplate;
    class ZIKLNKOBJ;
    class ZLNKOBJ;

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
