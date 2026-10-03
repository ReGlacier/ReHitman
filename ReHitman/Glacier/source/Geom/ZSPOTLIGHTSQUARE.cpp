#include <Glacier/Geom/ZSPOTLIGHTSQUARE.h>
#include <Glacier/Render/Prim/SPrimLightSpotSquare.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <cmath>


namespace Glacier
{
    ZSPOTLIGHTSQUARE::ZSPOTLIGHTSQUARE(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLIGHT(psName, pBaseGeom)
    {
        // Do nothing
    }

    ZSPOTLIGHTSQUARE::~ZSPOTLIGHTSQUARE() = default;

    const RTP::ZPropertyInfo& ZSPOTLIGHTSQUARE::GetProperties() const
    {
        return ZSPOTLIGHTSQUARE::Info;
    }

    uint32_t ZSPOTLIGHTSQUARE::GetObjectId() const
    {
        return ZSPOTLIGHTSQUARE::m_Id;
    }

    void ZSPOTLIGHTSQUARE::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSPOTLIGHTSQUARE::m_Id;
        mask = ZSPOTLIGHTSQUARE::m_Mask;
    }

    ZGEOMCLASSINFO* ZSPOTLIGHTSQUARE::GetOldClassInfo() const
    {
        return ZSPOTLIGHTSQUARE::m_OldClassInfo;
    }

    void ZSPOTLIGHTSQUARE::CalcCenSize()
    {
        if (auto lPrim = BaseGeom()->m_lPrim)
        {
            SPrimLightSpotSquare lightData;
            g_pRenderDll->m_pPrimControl->GetSpotLightSquareData(lPrim, &lightData);

            const float fRadius = std::cos(lightData.fFallOff) * lightData.fFarRange * std::tan(lightData.fFallOff);

            ZVector3 vFarEnd { fRadius, fRadius, lightData.fFarRange };
            ZVector3 vNearCorner { -fRadius, -fRadius, 0.0f };

            ZVector3 vCen = (vFarEnd + vNearCorner) * 0.5f;
            SetCen(vCen);

            ZVector3 vSize = vFarEnd - vCen;
            const float fBoundRadius = std::sqrt(vSize.x * vSize.x + vSize.y * vSize.y + vSize.z * vSize.z) + 1.0f;
            SetRadius(fBoundRadius);
            SetSize(vSize);
        }
    }

    void ZSPOTLIGHTSQUARE::SetMultiplier(float fValue)
    {
        MakeLightUnique();

        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        SPrimLightSpotSquare lightData;
        pPrimControl->GetSpotLightSquareData(BaseGeom()->m_lPrim, &lightData);

        if (lightData.fMultiplier != fValue)
        {
            lightData.fMultiplier = fValue;
            pPrimControl->ModifySpotLightSquare(BaseGeom()->m_lPrim, &lightData);
            m_lLightCon |= ZL_ATTRIBUTE_CHANGED;
        }
    }

    void ZSPOTLIGHTSQUARE::SetDiffuseColor(uint32_t lColor)
    {
        MakeLightUnique();

        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        SPrimLightSpotSquare lightData;
        pPrimControl->GetSpotLightSquareData(BaseGeom()->m_lPrim, &lightData);

        if (lightData.lDiffuseColor != lColor)
        {
            lightData.lDiffuseColor = lColor;
            pPrimControl->ModifySpotLightSquare(BaseGeom()->m_lPrim, &lightData);
            m_lLightCon |= ZL_ATTRIBUTE_CHANGED;
        }
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZSPOTLIGHTSQUARE,
        ZLIGHT,
        0x00972C08,
        "ZSPOTLIGHTSQUARE",
        0x0076BE40,
        nullptr,
        0x008143AC,
        0x00972AF0,
        0x00972AF4
    );
#   pragma endregion
}
