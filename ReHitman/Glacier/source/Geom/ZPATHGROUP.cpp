#include <Glacier/Geom/ZPATHGROUP.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZPATHGROUP::ZPATHGROUP(const char* psName, ZBaseGeom* pBaseGeom)
	    : ZGROUP(psName, pBaseGeom)
	{
	    m_prtLines = ZUniMemory::New<REFTAB>(8, 0);
	}

    ZPATHGROUP::~ZPATHGROUP()
    {
        if (m_prtLines)
        {
            for (const uint32_t lRef : *m_prtLines)
            {
                if (auto* pGeom = ZGEOM::RefToPtr(lRef))
                {
                    pGeom->Delete();
                }
            }

            ZUniMemory::Delete(m_prtLines);
            m_prtLines = nullptr;
        }
    }

    const RTP::ZPropertyInfo& ZPATHGROUP::GetProperties() const
    {
        return ZPATHGROUP::Info;
    }

    uint32_t ZPATHGROUP::GetObjectId() const
    {
        return ZPATHGROUP::m_Id;
    }

    void ZPATHGROUP::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZPATHGROUP::m_Id;
        mask = ZPATHGROUP::m_Mask;
    }

    ZGEOMCLASSINFO* ZPATHGROUP::GetOldClassInfo() const
    {
        return ZPATHGROUP::m_OldClassInfo;
    }

    void ZPATHGROUP::ClassInit()
    {
        Hide(true);
    }

    void ZPATHGROUP::CreatePath()
    {
        // Walk the child path geoms, collecting the root points of the first and the last.
        bool bFirst = true;
        int lCount = 0;
        ZVector3 vPoint;
        ZVector3 vLastPoint;

        for (ZBaseGeom* pCur = BaseGeom(); pCur; RecurGetNext(&pCur))
        {
            if (pCur == BaseGeom())
            {
                continue;
            }

            ++lCount;

            if (bFirst)
            {
                vPoint.Reset();
                pCur->GetRootPoint(vPoint);
                bFirst = false;
            }
            else
            {
                vLastPoint.Reset();
                pCur->GetRootPoint(vLastPoint);
                vPoint = vLastPoint;
            }
        }

        if (lCount == 1)
        {
            ZBaseGeom* pCur = BaseGeom();
            while (pCur == BaseGeom())
            {
                RecurGetNext(&pCur);
                if (!pCur)
                {
                    return;
                }
            }

            ZMat3x3 mMat;
            ZVector3 vPos;
            pCur->GetRootTM(mMat, vPos);
        }
    }

    void ZPATHGROUP::RemovePath()
    {
        if (m_prtLines)
        {
            for (const uint32_t lRef : *m_prtLines)
            {
                if (auto* pGeom = ZGEOM::RefToPtr(lRef))
                {
                    pGeom->Delete();
                }
            }
        }

        m_prtLines->Clear();
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZPATHGROUP,
        ZGROUP,
        0x00973230,
        "ZPATHGROUP",
        0x0076CF58,
        nullptr,
        0x00809DAC,
        0x009731E0,
        0x009731E4
    );
#   pragma endregion
}
