#include <Glacier/Geom/ZSPOTLIGHT.h>
#include <Glacier/Render/Prim/SPrimLightSpot.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/RTP/VirtualTables.h>
#include <cmath>


namespace Glacier
{
    ZSPOTLIGHT::ZSPOTLIGHT(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLIGHT(psName, pBaseGeom)
    {
        // Do nothing
    }

    ZSPOTLIGHT::~ZSPOTLIGHT() = default;

    const RTP::ZPropertyInfo& ZSPOTLIGHT::GetProperties() const
    {
        return ZSPOTLIGHT::Info;
    }

    uint32_t ZSPOTLIGHT::GetObjectId() const
    {
        return ZSPOTLIGHT::m_Id;
    }

    void ZSPOTLIGHT::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZSPOTLIGHT::m_Id;
        mask = ZSPOTLIGHT::m_Mask;
    }

    ZGEOMCLASSINFO* ZSPOTLIGHT::GetOldClassInfo() const
    {
        return ZSPOTLIGHT::m_OldClassInfo;
    }

    void ZSPOTLIGHT::CalcCenSize()
    {
        if (auto lPrim = BaseGeom()->m_lPrim)
        {
            SPrimLightSpot lightData;
            g_pRenderDll->m_pPrimControl->GetSpotLightData(lPrim, &lightData);

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

    void ZSPOTLIGHT::CopyData(const ZGEOM* Source)
    {
        ZLIGHT::CopyData(Source);
    }

    void ZSPOTLIGHT::SetMultiplier(float fValue)
    {
        MakeLightUnique();

        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        SPrimLightSpot lightData;
        pPrimControl->GetSpotLightData(BaseGeom()->m_lPrim, &lightData);

        if (lightData.fMultiplier != fValue)
        {
            lightData.fMultiplier = fValue;
            pPrimControl->ModifySpotLight(BaseGeom()->m_lPrim, &lightData);
            m_lLightCon |= ZL_ATTRIBUTE_CHANGED;
        }
    }

    void ZSPOTLIGHT::SetDiffuseColor(uint32_t lColor)
    {
        MakeLightUnique();

        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        SPrimLightSpot lightData;
        pPrimControl->GetSpotLightData(BaseGeom()->m_lPrim, &lightData);

        if (lightData.lDiffuseColor != lColor)
        {
            lightData.lDiffuseColor = lColor;
            pPrimControl->ModifySpotLight(BaseGeom()->m_lPrim, &lightData);
            m_lLightCon |= ZL_ATTRIBUTE_CHANGED;
        }
    }

    float ZSPOTLIGHT::GetHotSpotAngle()
    {
        SPrimLightSpot lightData;
        g_pRenderDll->m_pPrimControl->GetSpotLightData(BaseGeom()->m_lPrim, &lightData);
        return lightData.fHotSpot;
    }

    float ZSPOTLIGHT::GetFallOffAngle()
    {
        SPrimLightSpot lightData;
        g_pRenderDll->m_pPrimControl->GetSpotLightData(BaseGeom()->m_lPrim, &lightData);
        return lightData.fFallOff;
    }

    float ZSPOTLIGHT::GetNearRange()
    {
        SPrimLightSpot lightData;
        g_pRenderDll->m_pPrimControl->GetSpotLightData(BaseGeom()->m_lPrim, &lightData);
        return lightData.fNearRange;
    }

    float ZSPOTLIGHT::GetFarRange()
    {
        SPrimLightSpot lightData;
        g_pRenderDll->m_pPrimControl->GetSpotLightData(BaseGeom()->m_lPrim, &lightData);
        return lightData.fFarRange;
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZSPOTLIGHT,
        ZLIGHT,
        0x00972BB8,
        "ZSPOTLIGHT",
        0x0076BE34,
        nullptr,
        0x008143A0,
        0x00972AE8,
        0x00972AEC
    );
#   pragma endregion
}
