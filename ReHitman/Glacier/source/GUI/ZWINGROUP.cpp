#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/GUI/ZWinEvents.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>


namespace Glacier
{
    ZWINGROUP::ZWINGROUP(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
        , m_pSystem(nullptr)
        , m_bNoLineBreak(false)
    {
    }

    ZWINGROUP::~ZWINGROUP() = default;

    const RTP::ZPropertyInfo& ZWINGROUP::GetProperties() const
    {
        return ZWINGROUP::Info;
    }

    uint32_t ZWINGROUP::GetObjectId() const
    {
        return ZWINGROUP::m_Id;
    }

    void ZWINGROUP::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZWINGROUP::m_Id;
        mask = ZWINGROUP::m_Mask;
    }

    ZGEOMCLASSINFO* ZWINGROUP::GetOldClassInfo() const
    {
        return ZWINGROUP::m_OldClassInfo;
    }

    void ZWINGROUP::SetPos(const Glacier::ZVector3& vPos)
    {
        ZVector3 vOldPos;
        ZGEOM::GetPos(vOldPos);

        if (vPos != vOldPos)
        {
            ZGEOM::SetPos(vPos);

            for (auto* pBaseGeom = BaseGeom(); pBaseGeom; RecurGetNext(&pBaseGeom))
            {
                if (pBaseGeom->IsDerivedFrom<ZWINOBJ>())
                {
                    if (pBaseGeom->GetGeom())
                    {
                        auto* pWinObj = static_cast<ZWINOBJ*>(pBaseGeom->GetGeom());
                        pWinObj->SetModified(true);
                    }
                }
            }
        }
    }

    void ZWINGROUP::ClassInit()
    {
        const ZMat3x3 mMat
        {
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f
        };

        SetMat(mMat);
    }

    int32_t ZWINGROUP::ClassCommand(ZMSGID Msg, void* pData)
    {
        if (Msg == 0x8000)
        {
            auto* pEvent = reinterpret_cast<ZWMEVENT*>(pData);
            pEvent->Return = WndMessage(pEvent);
        }
        else
        {
            ZGEOM::ClassCommand(Msg, pData);
        }

        return 0;
    }

    void ZWINGROUP::CopyData(const ZGEOM* Source)
    {
        ZGROUP::CopyData(Source);
    }

    ZWINDOWS* ZWINGROUP::GetSystem()
    {
        if (!m_pSystem)
        {
            if (IsDerivedFrom<ZWINDOWS>())
            {
                m_pSystem = static_cast<ZWINDOWS*>(this);
            }
            else
            {
                ZGROUP* pParent = Parent();
                if (pParent && pParent->IsDerivedFrom<ZWINGROUP>())
                    m_pSystem = static_cast<ZWINGROUP*>(pParent)->GetSystem();
            }
        }

        return m_pSystem;
    }

    bool ZWINGROUP::WndMessage(struct ZWMEVENT* event)
    {
        return false;
    }

    void ZWINGROUP::GetMouseColi(SMouseColi& coli, const ZVector3& vPos, const ZMat3x3& mMat)
    {
        ZVector3 vChildPos;
        vadd(vChildPos, vPos, BaseGeom()->m_vPos);
        TransformRootVector(vChildPos, BaseGeom()->m_mMat);

        ZMat3x3 mChildMat;
        mmmul(mChildMat, mMat, BaseGeom()->m_mMat);

        for (ZBaseGeom* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
        {
            const uint32_t lControl = pBaseGeom->Control();
            if ((lControl & 0x400) != 0 || (!m_bNoLineBreak && (lControl & 0x800) != 0))
                continue;

            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (!pGeom)
                continue;

            if (pGeom->IsDerivedFrom<ZWINGROUP>())
            {
                if ((pGeom->Control() & 0x400) == 0)
                    static_cast<ZWINGROUP*>(pGeom)->GetMouseColi(coli, vChildPos, mChildMat);
            }
            else if (pGeom->IsDerivedFrom<ZWINOBJ>())
            {
                static_cast<ZWINOBJ*>(pGeom)->GetMouseColi(coli, vChildPos, mChildMat);
            }
        }
    }

    void ZWINGROUP::RecalcMaxMin()
    {
        BaseGeom()->SetControl(ZCHASMOVED, 0);
        g_pEngineData->UnlockMinMax();
        BaseGeom()->ForceCalcMaxMin();

        auto* pGroup = BaseGeom()->ParentGroup();
        if (pGroup && pGroup->IsDerivedFrom<ZWINGROUP>())
        {
            static_cast<ZWINGROUP*>(pGroup)->RecalcMaxMin();
        }

        g_pEngineData->LockMinMax();
    }

    void ZWINGROUP::CalcRealCenSize(ZVector3& vCen, ZVector3& vSize) const
    {
        ZVector3 vMin { 9.9999997e37f, 9.9999997e37f, 9.9999997e37f };
        ZVector3 vMax { -9.9999997e37f, -9.9999997e37f, -9.9999997e37f };

        for (ZBaseGeom* pBaseGeom = m_pGroupFirst; pBaseGeom; pBaseGeom = pBaseGeom->Next())
        {
            if ((pBaseGeom->Control() & 0xC00) != 0)
                continue;

            ZVector3 vChildCen;
            ZVector3 vChildSize;
            if (pBaseGeom->IsDerivedFrom<ZWINGROUP>())
            {
                static_cast<ZWINGROUP*>(pBaseGeom->GetGeom())->CalcRealCenSize(vChildCen, vChildSize);
            }
            else
            {
                vChildCen = pBaseGeom->m_vCen;
                vChildSize = pBaseGeom->m_vSize;
            }

            ZVector3 vChildMin;
            ZVector3 vChildMax;
            vsub(vChildMin, vChildCen, vChildSize);
            vadd(vChildMax, vChildCen, vChildSize);

            if (!pBaseGeom->IsDerivedFrom<ZWINGROUP>() && pBaseGeom->IsDerivedFrom<ZWINOBJ>())
            {
                const ZVector2& vAlignment = static_cast<ZWINOBJ*>(pBaseGeom->GetGeom())->m_vAlignmentOffset;
                vChildMin.x += vAlignment.x;
                vChildMin.y += vAlignment.y;
                vChildMax.x += vAlignment.x;
                vChildMax.y += vAlignment.y;
            }

            vadd(vChildMin, pBaseGeom->m_vPos);
            vadd(vChildMax, pBaseGeom->m_vPos);
            vmin(vMin, vChildMin);
            vmax(vMax, vChildMax);
        }

        vadd(vCen, vMax, vMin);
        vsub(vSize, vMax, vMin);
        vscalar(vCen, 0.5f);
        vscalar(vSize, 0.5f);
    }

    void ZWINGROUP::SetText(const char* text)
    {
        for (ZBaseGeom* pBaseGeom = m_pGroupLast; pBaseGeom && !ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZCHAROBJ>())
                static_cast<ZCHAROBJ*>(pGeom)->SetText(text);
        }
    }

    void ZWINGROUP::SetAlpha(int lAlpha)
    {
        for (ZBaseGeom* pBaseGeom = m_pGroupLast; pBaseGeom && !ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZWINOBJ>())
                static_cast<ZWINOBJ*>(pGeom)->SetAlpha(static_cast<uint8_t>(lAlpha));
        }
    }

    void ZWINGROUP::SetLineSpacing(int lSpacing)
    {
        for (ZBaseGeom* pBaseGeom = m_pGroupLast; pBaseGeom && !ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->GetPrev())
        {
            ZGEOM* pGeom = pBaseGeom->GetGeom();
            if (pGeom && pGeom->IsDerivedFrom<ZCHAROBJ>())
                static_cast<ZCHAROBJ*>(pGeom)->m_iLineSpacing = static_cast<int8_t>(lSpacing);
        }
    }

    void ZWINGROUP::ForceNoLineBreak(bool bLineNoBreak)
    {
        m_bNoLineBreak = bLineNoBreak;
    }

    void ZWINGROUP::SetPos(float x, float y, float z)
    {
        ZVector3 vNewPos { x, y, z };
        SetPos(vNewPos);
    }

#pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZWINGROUP,
        ZGROUP,
        0x009A28D8,
        "ZWINGROUP",
        0x0077CF98,
        nullptr,
        0x00806C0C,
        0x009A27D4,
        0x009A27D8);
#pragma endregion
}
