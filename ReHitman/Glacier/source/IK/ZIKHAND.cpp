#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Geom/ZROOM.h>


namespace Glacier
{
    SHandInfo::SHandInfo() = default;

    void SHandInfo::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        stream.Exchange("m_rItem", m_rItem);
        stream.Exchange("m_bIKItemEnabled", m_bIKItemEnabled);
    }

    ZIKHAND::ZIKHAND(uint32_t lBoneId)
        : ZTARGET()
        , m_vItemOffset(0.f)
        , m_lBoneId(lBoneId)
        , m_HandInfo()
    {
    }

    void ZIKHAND::LoadSave(ZLNKOBJ* pLnkObj, ISerializerStream& stream, bool bSaving)
    {
        ZTARGET::LoadSave(pLnkObj, stream, bSaving);
        m_HandInfo.LoadSave(stream, bSaving);

        stream.Exchange("m_lBoneId", m_lBoneId);
        stream.ExchangeArray("m_vItemOffset", m_vItemOffset, 3);
    }

    ZREF ZIKHAND::AttachItem(ZIKLNKOBJ* pLnkObj, ZREF rItem)
    {
        if (rItem && rItem == m_HandInfo.m_rItem)
        {
            ZERROR("NOTIFY: ZIKHAND::AttachItem tried to attach same item twice!");
            return m_HandInfo.m_rItem;
        }

        auto* pItem = reinterpret_cast<ZItem*>(ZGEOM::RefToPtr(rItem));
        if (pItem)
        {
            ZASSERT(!pItem->IsNew());

            m_HandInfo.m_bIKItemEnabled = true;
            const ZREF rRootRef = g_pEngineData->m_pRoot->GetRef();
            pItem->SetItemOwner(rRootRef, g_pEngineData->m_pRoot, true, true);

            ZMat3x3 mItemMat;
            ZVector3 vItemPos;
            pItem->GetMainMatPos(mItemMat, vItemPos, m_lBoneId);
            pLnkObj->AttachBaseGeomToBone(pItem->BaseGeom(), m_lBoneId, mItemMat, vItemPos);
            pItem->BaseGeom()->SetOwnerDraw(true);
        }
        else
        {
            auto* pDetachItem = reinterpret_cast<ZItem*>(ZGEOM::RefToPtr(m_HandInfo.m_rItem));
            if (pDetachItem)
            {
                pLnkObj->DetachBaseGeomFromBone(pDetachItem->BaseGeom(), m_lBoneId);
                pDetachItem->BaseGeom()->SetOwnerDraw(false);
                PlaceItem(pLnkObj, pDetachItem);
                m_HandInfo.m_bIKItemEnabled = false;
                rItem = 0;
            }
            else
            {
                ZASSERT(!m_HandInfo.m_bIKItemEnabled);
                return 0;
            }
        }

        if (rItem)
            pLnkObj->ClassCommand(m_msgInventorySetActive, reinterpret_cast<void*>(rItem));
        else
            pLnkObj->ClassCommand(m_msgInventorySetActive, reinterpret_cast<void*>(2));

        const ZREF oldRef = m_HandInfo.m_rItem;
        m_HandInfo.m_rItem = rItem;
        return oldRef;
    }

    void ZIKHAND::SlipItem(ZIKLNKOBJ* pLnkObj)
    {
        if (GetItem())
            AttachItem(pLnkObj, 0);
    }

    ZItem* ZIKHAND::GetItem() const
    {
        if (m_HandInfo.m_bIKItemEnabled)
        {
            // It's pretty unsafe, but in original code no checks for RTTI stubs
            return reinterpret_cast<ZItem*>(ZGEOM::RefToPtr(m_HandInfo.m_rItem));
        }

        return nullptr;
    }

    ITEMHANDS ZIKHAND::GetItemType() const
    {
        if (auto* pItem = GetItem())
        {
            if (auto* pItemTemplate = pItem->GetItemTemplate())
            {
                return pItemTemplate->GetItemHands();
            }
        }

        return ITEMHANDS::IH_NONE;
    }

    void ZIKHAND::Reset()
    {
        Clear();
        m_HandInfo.m_rItem = 0;
    }

    void ZIKHAND::SetTarget(ZIKLNKOBJ* pLnkObj, const ZMat3x3& mMat, const ZVector3& vPos, float fTime, ZIKCALLBACK CallBack)
    {
        ZTARGET::CallBackAndRemove(pLnkObj);
        ZTARGET::SetMatPosGeom(&mMat, &vPos, 0);
        ZTARGET::SetCallBack(CallBack);
        ZTARGET::SetTime(fTime, false);
        ZTARGET::Enable(true);
    }

    void ZIKHAND::PlaceItem(ZIKLNKOBJ* pLnkObj, ZItem* _pItem)
    {
        const auto lBoneNr = pLnkObj->GetBoneNrFromId(m_lBoneId);

        ZMat3x3 mBoneMat;
        ZVector3 vBonePos;
        pLnkObj->GetIKBoneMatPos(lBoneNr, mBoneMat, vBonePos);

        ZMat3x3 mMat;
        ZVector3 vPos;
        pLnkObj->GetRootMatPos(mMat, vPos);

        auto* pParentGroup = _pItem->BaseGeom()->ParentGroup();

        ZMat3x3 mParentMat;
        ZVector3 vParentPos;
        pParentGroup->GetLocalMatPos(mParentMat, vParentPos);

        ZVector3 vOffset;
        vmmul(vOffset, m_vItemOffset, mParentMat);
        vadd(vPos, vOffset);

        _pItem->Place(mParentMat, vPos);
    }
}
