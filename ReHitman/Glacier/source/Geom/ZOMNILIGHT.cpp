#include <Glacier/Geom/ZOMNILIGHT.h>
#include <Glacier/Render/Prim/SPrimLightOmni.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    ZOMNILIGHT::ZOMNILIGHT(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLIGHT(psName, pBaseGeom)
    {
        // Do nothing
    }

    ZOMNILIGHT::~ZOMNILIGHT() = default;

    // RTP::cBase
    const RTP::ZPropertyInfo& ZOMNILIGHT::GetProperties() const
    {
        return ZOMNILIGHT::Info;
    }

    uint32_t ZOMNILIGHT::GetObjectId() const
    {
        return ZOMNILIGHT::m_Id;
    }

    void ZOMNILIGHT::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZOMNILIGHT::m_Id;
        mask = ZOMNILIGHT::m_Mask;
    }

    ZGEOMCLASSINFO* ZOMNILIGHT::GetOldClassInfo() const
    {
        return ZOMNILIGHT::m_OldClassInfo;
    }

    void ZOMNILIGHT::SetMultiplier(float fValue)
    {
        MakeLightUnique();

        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        SPrimLightOmni omniData;
        pPrimControl->GetOmniLightData(BaseGeom()->m_lPrim, &omniData);

        if (omniData.fMultiplier != fValue)
        {
            omniData.fMultiplier = fValue;
            pPrimControl->ModifyOmniLight(BaseGeom()->m_lPrim, &omniData);
            m_lLightCon |= ZL_ATTRIBUTE_CHANGED;
        }
    }

    void ZOMNILIGHT::SetDiffuseColor(uint32_t lColor)
    {
        MakeLightUnique();

        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        SPrimLightOmni omniData;
        pPrimControl->GetOmniLightData(BaseGeom()->m_lPrim, &omniData);

        if (omniData.lDiffuseColor != lColor)
        {
            omniData.lDiffuseColor = lColor;
            pPrimControl->ModifyOmniLight(BaseGeom()->m_lPrim, &omniData);
        }
    }

    void ZOMNILIGHT::SetNearRange(float value)
    {
        MakeLightUnique();

        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        SPrimLightOmni omniData;
        pPrimControl->GetOmniLightData(BaseGeom()->m_lPrim, &omniData);

        if (omniData.fNearRange != value)
        {
            omniData.fNearRange = value;
            pPrimControl->ModifyOmniLight(BaseGeom()->m_lPrim, &omniData);
        }
    }

    void ZOMNILIGHT::SetFarRange(float value)
    {
        MakeLightUnique();

        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        SPrimLightOmni omniData;
        pPrimControl->GetOmniLightData(BaseGeom()->m_lPrim, &omniData);

        if (omniData.fFarRange != value)
        {
            omniData.fFarRange = value;
            pPrimControl->ModifyOmniLight(BaseGeom()->m_lPrim, &omniData);
        }
    }

    float ZOMNILIGHT::GetNearRange()
    {
        SPrimLightOmni omniData;
        g_pRenderDll->m_pPrimControl->GetOmniLightData(BaseGeom()->m_lPrim, &omniData);
        return omniData.fNearRange;
    }

    float ZOMNILIGHT::GetFarRange()
    {
        SPrimLightOmni omniData;
        g_pRenderDll->m_pPrimControl->GetOmniLightData(BaseGeom()->m_lPrim, &omniData);
        return omniData.fFarRange;
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZOMNILIGHT,
        ZLIGHT,
        0x00972C58,
        "ZOMNILIGHT",
        0x0076BE54,
        nullptr,
        0x008143B8,
        0x00972AF8,
        0x00972AFC
    );
#   pragma endregion
}
