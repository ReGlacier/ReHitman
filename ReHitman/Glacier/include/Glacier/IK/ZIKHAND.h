#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/IK/ZTARGET.h>
#include <Glacier/Items/ZItemTemplate.h>


namespace Glacier
{
    // fwds
    class ZItem;
    class ZItemTemplate;
    class ZIKLNKOBJ;
    class ZLNKOBJ;

    struct SHandInfo
    {
        // methods
        SHandInfo();

        void LoadSave(ISerializerStream& stream, bool bSaving);

        // members
        uint32_t m_rItem{ 0 };
        bool m_bIKItemEnabled{ false };
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(SHandInfo, 0x8);

    class ZIKHAND : public ZTARGET
    {
    public:
        // vtbl
        void LoadSave(ZLNKOBJ* pLnkObj, ISerializerStream& stream, bool bSaving) override;
        virtual ZREF AttachItem(ZIKLNKOBJ* pLnkObj, ZREF rItem);
        virtual void SlipItem(ZIKLNKOBJ* pLnkObj);
        virtual ZItem* GetItem() const;
        virtual ITEMHANDS GetItemType() const;
        virtual void Reset();
        virtual void SetTarget(ZIKLNKOBJ* pLnkObj, const ZMat3x3& mMat, const ZVector3& vPos, float fTime, ZIKCALLBACK CallBack);

        // methods
        ZIKHAND(uint32_t lBoneId);
        void PlaceItem(ZIKLNKOBJ* pLnkObj, ZItem* _pItem);

        // data
        SHandInfo m_HandInfo;
        unsigned int m_lBoneId;
        Glacier::ZVector3 m_vItemOffset;
        ZMSGID m_msgInventorySetActive;
        uint8_t m_pad2[6];
    };
    RE_VERIFY_SIZE(ZIKHAND, 0x80); // Verified for PC
}
