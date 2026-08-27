#include <Glacier/Geom/ZSHAPE.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/RTP/VirtualTables.h>
#include <cmath>
#include <new>


namespace Glacier
{
    bool BasicShape::Save(struct ZSaveGame*)
    {
        if (m_pSegments)
        {
            ZUniMemory::Delete(m_pSegments);
            m_pSegments = nullptr;
        }

        return true;
    }

    bool BasicShape::Load(struct ZLoadGame*)
    {
        return true;
    }

    bool BasicShape::IsNormalized() const
    {
        bool bResult = true;

        const float fAvgLen = GetLen() / static_cast<float>(m_lNrSegment);
        for (int i = 0; i < m_lNrSegment; ++i)
        {
            if (std::fabs(fAvgLen - m_pSegments[i].GetLen()) > fAvgLen * 0.25f)
            {
                bResult = false;
            }
        }

        return bResult;
    }

    void BasicShape::DefineBezierSpline(ParametricCurve* pCurves, const float (*aVertices)[3], int lNrSegments, float fTess)
    {
        m_lNrSegment = lNrSegments;
        m_pSegments = pCurves;

        if (m_lNrSegment && m_pSegments && aVertices)
        {
            m_fLen = 0.0f;
            m_fMinSegLen = 9.9999997e37f;

            for (int i = 0; i < m_lNrSegment; ++i)
            {
                m_pSegments[i].DefineBezier(aVertices, aVertices + 1, aVertices + 2, aVertices + 3, fTess);
                m_fLen += m_pSegments[i].GetLen();
                if (m_pSegments[i].GetLen() < m_fMinSegLen)
                {
                    m_fMinSegLen = m_pSegments[i].GetLen();
                }

                aVertices += 3;
            }
        }
    }

    float BasicShape::GetLen() const
    {
        if (m_fLen < 0.0f)
        {
            const_cast<BasicShape*>(this)->m_fLen = CalcShapeLen();
        }

        return m_fLen;
    }

    float BasicShape::CalcShapeLen() const
    {
        float fLen = 0.0f;
        for (int i = 0; i < m_lNrSegment; ++i)
        {
            fLen += m_pSegments[i].GetLen();
        }

        const_cast<BasicShape*>(this)->m_fLen = fLen;
        return fLen;
    }

    void BasicShape::GetSplinePos(float (*pPos)[3], float t) const
    {
        int lSegment = static_cast<int>(static_cast<float>(m_lNrSegment) * t);
        if (lSegment >= m_lNrSegment)
        {
            lSegment = m_lNrSegment - 1;
        }

        m_pSegments[lSegment].GetSplinePos(pPos, t);
    }

    void BasicShape::GetSplineVelocity(float (*pVel)[3], float t) const
    {
        int lSegment = static_cast<int>(static_cast<float>(m_lNrSegment) * t);
        if (lSegment >= m_lNrSegment)
        {
            lSegment = m_lNrSegment - 1;
        }

        m_pSegments[lSegment].GetSplineVelocity(pVel, t);
    }

    void BasicShape::GetSplinePosVelocity(float (*pPos)[3], float (*pVel)[3], float t) const
    {
        int lSegment = static_cast<int>(static_cast<float>(m_lNrSegment) * t);
        if (lSegment >= m_lNrSegment)
        {
            lSegment = m_lNrSegment - 1;
        }

        m_pSegments[lSegment].GetSplinePos(pPos, t);
        m_pSegments[lSegment].GetSplineVelocity(pVel, t);
    }

    BasicShape::BasicShape()
    {
        m_fLen = -1.0f;
        m_fMinSegLen = 9.9999997e37f;
        m_pSegments = nullptr;
        m_lNrSegment = 0;
    }

    ZSHAPE::ZSHAPE(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGEOM(psName, pBaseGeom)
        , m_S()
    {
        m_pShapeSegments = nullptr;
    }

    ZSHAPE::~ZSHAPE()
    {
        if (m_pShapeSegments)
        {
            --m_pShapeSegments->m_iReferences;
            if (m_pShapeSegments->m_iReferences == 0)
            {
                ZUniMemory::Delete(m_pShapeSegments);
            }
        }
    }

    bool ZSHAPE::PostLoad(ISerializerStream& stream)
    {
        if (m_pShapeSegments)
        {
            --m_pShapeSegments->m_iReferences;
            if (m_pShapeSegments->m_iReferences == 0)
            {
                ZUniMemory::Delete(m_pShapeSegments);
            }
            m_pShapeSegments = nullptr;
        }

        if (auto lPrim = BaseGeom()->m_lPrim)
        {
            auto* pIndices = g_pRenderDll->m_pPrimControl->GetPrimSplineIndices(lPrim);
            if (pIndices)
            {
                const int lNrSegments = static_cast<int>(*pIndices) - 1;
                auto* pShapeSegments = static_cast<SShapeParametricCurve*>(
                    ZUniMemory::Allocate(sizeof(SShapeParametricCurve) + sizeof(ParametricCurve) * (lNrSegments - 1)));

                m_pShapeSegments = pShapeSegments;
                pShapeSegments->m_iReferences = 1;

                for (int i = 0; i < lNrSegments; ++i)
                {
                    new (&pShapeSegments->m_aCurves[i]) ParametricCurve();
                }

                auto* pVertices = g_pRenderDll->m_pPrimControl->GetPrimSplineVertices(lPrim);
                m_S.DefineBezierSpline(&pShapeSegments->m_aCurves[0], reinterpret_cast<const float(*)[3]>(pVertices), lNrSegments, 0.1f);
                m_S.IsNormalized();
            }
        }

        return true;
    }

    const RTP::ZPropertyInfo& ZSHAPE::GetProperties() const
    {
        return ZSHAPE::Info;
    }

    uint32_t ZSHAPE::GetObjectId() const
    {
        return ZSHAPE::m_Id;
    }

    void ZSHAPE::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSHAPE::m_Id;
        mask = ZSHAPE::m_Mask;
    }

    ZGEOMCLASSINFO* ZSHAPE::GetOldClassInfo() const
    {
        return ZSHAPE::m_OldClassInfo;
    }

    void ZSHAPE::CalcCenSize()
    {
        BaseGeom()->CalcCenSize(true);
    }

    void ZSHAPE::CopyData(const ZGEOM* Source)
    {
        ZGEOM::CopyData(Source);

        if (const auto* pSource = geom_cast<ZSHAPE>(Source))
        {
            m_pShapeSegments = pSource->m_pShapeSegments;
            if (m_pShapeSegments && !g_pEngineData->CheckInPackBuffer(m_pShapeSegments))
            {
                ++m_pShapeSegments->m_iReferences;
            }
        }
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZSHAPE,
        ZGEOM,
        0x0097B5F0,
        "ZSHAPE",
        0x0076EBB0,
        nullptr,
        0x0080A968,
        0x0097B5A0,
        0x0097B5A4
    );
#   pragma endregion
}
