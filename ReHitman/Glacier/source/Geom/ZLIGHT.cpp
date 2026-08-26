#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZSTL/CListUser.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <algorithm>


namespace Glacier
{
    ZLIGHT::ZLIGHT(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGEOM(psName, pBaseGeom)
    {
        m_lLightCon = 0;
        m_pExcludeList = nullptr;
        m_pStaticShadowList = nullptr;
        if (g_pEngineData->m_bLightDisplay)
            SetGeomControl(1u, 0u);
    }

    ZLIGHT::~ZLIGHT()
    {
        ActivateStaticShadows(false);
        BaseGeom()->LightNotifyPotentialDetachment(false);

        // DronCode; In original code here we have memory leak, but I fixed that
        if (m_pExcludeList)
        {
            ZUniMemory::Free(m_pExcludeList);
            m_pExcludeList = nullptr;
        }

        if (m_pStaticShadowList)
        {
            ZUniMemory::Free(m_pStaticShadowList);
            m_pStaticShadowList = nullptr;
        }
    }

    bool ZLIGHT::PostLoad(ISerializerStream& stream)
    {
        ZSerializable::PostLoad(stream);
        return true;
    }

    const RTP::ZPropertyInfo& ZLIGHT::GetProperties() const
    {
        return ZLIGHT::Info;
    }

    uint32_t ZLIGHT::GetObjectId() const
    {
        return ZLIGHT::m_Id;
    }

    void ZLIGHT::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZLIGHT::m_Id;
        mask = ZLIGHT::m_Mask;
    }

    ZGEOMCLASSINFO* ZLIGHT::GetOldClassInfo() const
    {
        return ZLIGHT::m_OldClassInfo;
    }

    void ZLIGHT::Activate(bool bActive)
    {
        ZGEOM::Active();
        SetLightAttributeChanged(true);

        if (BaseGeom()->m_uListID != 0u)
        {
            if (bActive)
            {
                BaseGeom()->SetControl(0x3000000u, ZCINVIEW);
                if (auto* pList = g_pEngineData->GetListUser())
                    pList->NotifyAllMembers(BaseGeom());

                ActivateStaticShadows(true);
            }
            else
            {
                BaseGeom()->SetControl(0x1000000 | ZCINACTIVE, 0);
                if (auto* pList = g_pEngineData->GetListUser())
                    pList->NotifyAllMembers(BaseGeom());

                ActivateStaticShadows(false);
            }
        }
    }

    void ZLIGHT::CalcCenSize()
    {
        if (!Prim())
        {
            return;
        }

        ZVector3 vCenter, vSize;
        ZPrimControlBase::Instance()->CalcPrimCenSize(Prim(), vCenter, vSize, true);
        const float fRadius = vlen(vSize) + 1.0f;
        SetRadius(fRadius);
    }

    bool ZLIGHT::DisableParentBoundAdjust()
    {
        return (LightCon() & ZL_USE_PARENT_BOUNT_ADJUST) == 0;
    }

    void ZLIGHT::ClassInit()
    {
        if (m_pExcludeList)
        {
            auto* pListUser = g_pEngineData->GetListUser();
            for (int i = 0; i < *m_pExcludeList; ++i)
            {
                ZREF rLightRef = m_pExcludeList[i + 1];
                auto* pGeom = ZGEOM::RefToPtr(rLightRef);
                if (pGeom)
                {
                    pListUser->DisconnectNodeFromNode(BaseGeom(), pGeom->BaseGeom());
                }
            }
        }

        // NOTE: Maybe we need to do same with m_pStaticShadowList? In PC - do nothing
    }

    void ZLIGHT::CopyData(const ZGEOM* Source)
    {
        ZGEOM::CopyData(Source);

        if (auto* pSource = geom_cast<ZLIGHT>(Source))
        {
            m_lLightCon = pSource->m_lLightCon;
            if (g_pEngineData->m_bLightDisplay)
                SetGeomControl(1u, 0u);
        }
    }

    eGlobalTreeType ZLIGHT::GetBoundTreeType() const
    {
        return eGlobalTreeType::GT_Lights;
    }

    void ZLIGHT::Enable()
    {
        BaseGeom()->SetControl(0, ZCINVISIBLE);
        SetLightCon(ZL_ATTRIBUTE_CHANGED, 9);
    }

    void ZLIGHT::Disable()
    {
        BaseGeom()->SetControl(ZCINVISIBLE, 0);
        SetLightCon(9, 0);
    }

    void ZLIGHT::SetMultiplier(float fValue)
    {
        // Do nothing
    }

    void ZLIGHT::SetDiffuseColor(uint32_t lColor)
    {
        // Do nothing
    }

    void ZLIGHT::MakeLightUnique()
    {
        if (!ZPrimControlBase::Instance()->IsPrimUnique(Prim()))
        {
            auto lNewPrim = ZPrimControlBase::Instance()->CopyPrim(Prim(), 0u);
            BaseGeom()->SetPrim(lNewPrim);
        }
    }

    void ZLIGHT::SetLightCon(uint32_t lAdd, uint32_t lRemove)
    {
        m_lLightCon = lAdd | (m_lLightCon & ~lRemove);
    }

    uint32_t ZLIGHT::LightCon() const
    {
        return m_lLightCon;
    }

    void ZLIGHT::ActivateStaticShadows(bool bActive)
    {
        if (m_pStaticShadowList)
        {
            for (int i = 0; i < *m_pStaticShadowList; ++i)
            {
                auto* pShadowGeom = ZGEOM::RefToPtr(m_pStaticShadowList[i + 1]);
                if (pShadowGeom)
                {
                    if (bActive)
                    {
                        pShadowGeom->MakeActive();
                    }
                    else
                    {
                        pShadowGeom->MakeInactive();
                    }
                }
            }
        }
    }

    void ZLIGHT::SetLightAttributeChanged(bool bChanged)
    {
        SetLightCon(bChanged ? ZL_ATTRIBUTE_CHANGED : 0, bChanged ? 0 : ZL_ATTRIBUTE_CHANGED);
    }

    void ZLIGHT::SetEffectLight(bool bEffect)
    {
        SetLightCon(bEffect ? ZL_EFFECTS_LIGHT : 0, bEffect ? 0 : ZL_EFFECTS_LIGHT);
    }

    bool ZLIGHT::LightAttributeChanged() const
    {
        return (LightCon() & ZL_ATTRIBUTE_CHANGED) != 0;
    }

    bool ZLIGHT::EffectLight() const
    {
        return (LightCon() & ZL_EFFECTS_LIGHT) != 0;
    }

    bool ZLIGHT::IsGeomExcluded(ZREF rGeom) const
    {
        // DronCode: in original PS2 code used special function 'binarySearch',
        //           luckily we've in 2k26 we have something like that in STL.
        if (!m_pExcludeList || m_pExcludeList[0] == 0)
            return false;

        const uint32_t* begin = m_pExcludeList + 1;
        const uint32_t* end   = m_pExcludeList + 1 + m_pExcludeList[0];

        return std::binary_search(begin, end, rGeom);
    }

    void ZLIGHT::GetParentBound(bool& bUseParentBound)
    {
        bUseParentBound = (LightCon() & ZL_USE_PARENT_BOUNT_ADJUST) != 0;
    }

    void ZLIGHT::SetParentBound(const bool& bUseParentBound)
    {
        SetLightCon(bUseParentBound ? ZL_USE_PARENT_BOUNT_ADJUST : 0, bUseParentBound ? 0 : ZL_USE_PARENT_BOUNT_ADJUST);
    }

    void ZLIGHT::GetExcludeList(REFTAB32& aExcludeList)
    {
        // Do nothing
    }

    void ZLIGHT::SetExcludeList(const REFTAB32& aExcludeList)
    {
        if (!aExcludeList.Count())
        {
            return;
        }

        if (m_pExcludeList)
        {
            ZUniMemory::Free(m_pExcludeList);
        }

        m_pExcludeList = (uint32_t*)ZUniMemory::Allocate(sizeof(uint32_t) * aExcludeList.Count() + 1);
        uint32_t lInsertIndex = 1u;

        for (auto it = aExcludeList.As<ZREF>().begin(); it != aExcludeList.As<ZREF>().end();)
        {
            auto* pGeom = ref_cast<ZGEOM>(*it);
            if (pGeom)
            {
                m_pExcludeList[lInsertIndex++] = *it;
                ++it;
            }
            else
            {
                it.Erase();
            }
        }

        const int lElementsCount = lInsertIndex - 1;
        *m_pExcludeList = lElementsCount;
        qsort(
            (void*)(m_pExcludeList + 1),
            lElementsCount,
            sizeof(uint32_t),
            [](const void* a, const void* b) -> int
            {
                const uint32_t lLeft = *reinterpret_cast<const uint32_t*>(a);
                const uint32_t lRight = *reinterpret_cast<const uint32_t*>(b);

                return lRight <= lLeft ? lRight < lLeft : -1;
            });
    }

    void ZLIGHT::GetStaticShadowList(REFTAB32& aStaticList)
    {
        // Do nothing
    }

    void ZLIGHT::SetStaticShadowList(const REFTAB32& aStaticList)
    {
        if (!aStaticList.Count())
        {
            return;
        }

        if (m_pStaticShadowList)
        {
            ZUniMemory::Free(m_pStaticShadowList);
        }

        m_pStaticShadowList = (uint32_t*)ZUniMemory::Allocate(sizeof(uint32_t) * aStaticList.Count() + 1);
        uint32_t lInsertIndex = 1u;

        for (auto it = aStaticList.As<ZREF>().begin(); it != aStaticList.As<ZREF>().end();)
        {
            auto* pGeom = ref_cast<ZGEOM>(*it);
            if (pGeom)
            {
                m_pExcludeList[lInsertIndex++] = *it;
                ++it;
            }
            else
            {
                it.Erase();
            }
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<REFTAB32> NamespaceItem_398A
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "StaticShadowList",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__4,
            .m_Get = &ZLIGHT::GetStaticShadowList,
            .m_Set = &ZLIGHT::SetStaticShadowList
        };

        static RTP::ZVirtualProperty<REFTAB32> NamespaceItem_3989
        {
            .m_Node = {
                .m_Next = NamespaceItem_398A,
                .m_Name = "ExcludeList",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__4,
            .m_Get = &ZLIGHT::GetExcludeList,
            .m_Set = &ZLIGHT::SetExcludeList
        };

        static RTP::ZVirtualProperty<bool> NamespaceItem_3988
        {
            .m_Node = {
                .m_Next = NamespaceItem_3989,
                .m_Name = "ParentBound",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZLIGHT::GetParentBound,
            .m_Set = &ZLIGHT::SetParentBound
        };

        static RTP::ZDataProperty<uint32_t> NamespaceItem_3985
        {
            .m_Node = {
                .m_Next = NamespaceItem_3988,
                .m_Name = "m_lLightCon",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZLIGHT, m_lLightCon)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZLIGHT,
        ZGEOM,
        0x00972B68,
        "ZLIGHT",
        0x0076A148,
        cProperties::NamespaceItem_3985,
        0x00806854,
        0x00972AE0,
        0x00972AE4
    );
#   pragma endregion
}
