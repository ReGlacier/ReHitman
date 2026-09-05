#include <Glacier/Geom/ZENVIRONMENT.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Render/Prim/SPrimLightEnvironment.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/SPrimLight.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/ZSTL/StringUtils.h>


namespace Glacier
{
    ZENVIRONMENT::ZENVIRONMENT(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLIGHT(psName, pBaseGeom)
    {
        SetLightCon(0x3000u, 0);
        m_pCharacterLight = this;
        m_pCharacterShadow = this;
    }

    ZENVIRONMENT::~ZENVIRONMENT() = default;

    const RTP::ZPropertyInfo& ZENVIRONMENT::GetProperties() const
    {
        return ZENVIRONMENT::Info;
    }

    uint32_t ZENVIRONMENT::GetObjectId() const
    {
        return ZENVIRONMENT::m_Id;
    }

    void ZENVIRONMENT::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZENVIRONMENT::m_Id;
        mask = ZENVIRONMENT::m_Mask;
    }

    ZGEOMCLASSINFO* ZENVIRONMENT::GetOldClassInfo() const
    {
        return ZENVIRONMENT::m_OldClassInfo;
    }

    void ZENVIRONMENT::CalcCenSize()
    {
        ZVector3 v0;
        SetCen(v0);
        SetSize(v0);
        SetRadius(0.0f);
    }

    void ZENVIRONMENT::ClassInit()
    {
        if ((LightCon() & 0xC000) != 0)
            return;

        auto* pParentGroup = BaseGeom()->ParentGroup();
        for (auto* pEnvLight = pParentGroup->FindGeom("*_env_character*", nullptr); pEnvLight; pEnvLight = pParentGroup->FindGeom("*_env_character*", pEnvLight->BaseGeom()))
        {
            ZASSERT(pEnvLight->IsDerivedFrom<ZENVIRONMENT>());

            auto* pEnv = static_cast<ZENVIRONMENT*>(pEnvLight);
            if ((pEnv->LightCon() & ZLE_CHARACTER_LIGHT) != 0)
            {
                m_pCharacterLight = pEnv;
            }

            if ((pEnv->LightCon() & ZLE_CHARACTER_SHADOW) != 0)
            {
                m_pCharacterShadow = pEnv;

                // DronCode: It's another special hack for Hitman Blood Money. In other games there are simplified version of code
                const bool bIsM00 = striwcmp(g_pEngineData->GetSceneName(), "*M00_MAIN.zip") == 0;
                const bool bIsM01 = striwcmp(g_pEngineData->GetSceneName(), "*M01_MAIN.zip") == 0;
                const bool bIsM04 = striwcmp(g_pEngineData->GetSceneName(), "*M04_MAIN.zip") == 0;
                const bool bIsM05 = striwcmp(g_pEngineData->GetSceneName(), "*M05_MAIN.zip") == 0;

                if (bIsM00 || bIsM01 || bIsM04 || bIsM05)
                {
                    auto* pPrimSelf = ZPrimControlBase::GetPrimitive<SPrimLightEnvironment>(Prim());
                    auto* pPrimEnv  = ZPrimControlBase::GetPrimitive<SPrimLightEnvironment>(pEnv->Prim());

                    pPrimEnv->lStaticShadowColor = pPrimSelf->lStaticShadowColor;
                }
            }
        }

        if (!m_OnColor.m_Value || !m_OffColor.m_Value)
        {
            SPrimLightEnvironment sEnv {};
            ZPrimControlBase::Instance()->GetEnvironmentLightData(Prim(), &sEnv);

            if (!m_OnColor.m_Value)
            {
                m_OnColor.m_Value = sEnv.lDiffuseColor;
            }

            if (!m_OnBackColor.m_Value)
            {
                m_OnBackColor.m_Value = sEnv.lDiffuseColorBack;
            }

            if (!m_OffColor.m_Value)
            {
                m_OffColor.m_Value = sEnv.lDiffuseColor;
            }

            if (!m_OffBackColor.m_Value)
            {
                m_OffBackColor.m_Value = sEnv.lDiffuseColorBack;
            }
        }
    }

    void ZENVIRONMENT::CopyData(const ZGEOM* Source)
    {
        ZLIGHT::CopyData(Source);
    }

    void ZENVIRONMENT::SetMultiplier(float fValue)
    {
        MakeLightUnique();

        SPrimLightEnvironment sEnv {};
        ZPrimControlBase::Instance()->GetEnvironmentLightData(Prim(), &sEnv);
        sEnv.fMultiplier = fValue;
        ZPrimControlBase::Instance()->ModifyEnvironment(Prim(), &sEnv);
        SetLightAttributeChanged(true);
    }

    void ZENVIRONMENT::SetDiffuseColor(uint32_t lColor)
    {
        MakeLightUnique();

        SPrimLightEnvironment sEnv {};
        ZPrimControlBase::Instance()->GetEnvironmentLightData(Prim(), &sEnv);
        sEnv.lDiffuseColor = lColor;
        ZPrimControlBase::Instance()->ModifyEnvironment(Prim(), &sEnv);
        SetLightAttributeChanged(true);
    }

    void ZENVIRONMENT::ToggleColor(bool bOnOff)
    {
        if (bOnOff)
        {
            SetDiffuseColor(m_OnColor.m_Value, m_OnBackColor.m_Value);
        }
        else
        {
            SetDiffuseColor(m_OffColor.m_Value, m_OffBackColor.m_Value);
        }
    }

    void ZENVIRONMENT::SetDiffuseColor(uint32_t dwColor, uint32_t dwBackColor)
    {
        MakeLightUnique();

        SPrimLightEnvironment sEnv {};
        ZPrimControlBase::Instance()->GetEnvironmentLightData(Prim(), &sEnv);

        sEnv.lDiffuseColor = dwColor;
        sEnv.lDiffuseColorBack = dwBackColor;

        ZPrimControlBase::Instance()->ModifyEnvironment(Prim(), &sEnv);
        SetLightAttributeChanged(true);
    }

    void ZENVIRONMENT::GetAmbientEnable(bool& bEnable)
    {
        bEnable = (LightCon() & ZLE_AMBIENT_LIGHT) != 0;
    }

    void ZENVIRONMENT::SetAmbientEnable(const bool& bEnable)
    {
        SetLightCon(bEnable ? ZLE_AMBIENT_LIGHT : 0, bEnable ? 0 : ZLE_AMBIENT_LIGHT);
    }

    void ZENVIRONMENT::GetCharacterLight(bool& bLight)
    {
        bLight = (LightCon() & ZLE_CHARACTER_LIGHT) != 0;
    }

    void ZENVIRONMENT::SetCharacterLight(const bool& bLight)
    {
        SetLightCon(bLight ? ZLE_CHARACTER_LIGHT : 0, bLight ? 0 : ZLE_CHARACTER_LIGHT);
    }

    void ZENVIRONMENT::GetCharacterShadow(bool& bShadow)
    {
        bShadow = (LightCon() & ZLE_CHARACTER_SHADOW) != 0;
    }

    void ZENVIRONMENT::SetCharacterShadow(const bool& bShadow)
    {
        SetLightCon(bShadow ? ZLE_CHARACTER_SHADOW : 0, bShadow ? 0 : ZLE_CHARACTER_SHADOW);
    }

    void ZENVIRONMENT::GetFogEnable(bool& bEnable)
    {
        bEnable = (LightCon() & ZLE_FOG) != 0;
    }

    void ZENVIRONMENT::SetFogEnable(const bool& bEnable)
    {
        SetLightCon(bEnable ? ZLE_FOG : 0, bEnable ? 0 : ZLE_FOG);
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<ZCOLOR> NamespaceItem_A
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_OffBackColor",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__2,
            .m_Offset = CLASS_PROPERTY(ZENVIRONMENT, m_OffBackColor)
        };

        static RTP::ZDataProperty<ZCOLOR> NamespaceItem_9
        {
            .m_Node = {
                .m_Next = NamespaceItem_A,
                .m_Name = "m_OffColor",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__2,
            .m_Offset = CLASS_PROPERTY(ZENVIRONMENT, m_OffColor)
        };

        static RTP::ZDataProperty<ZCOLOR> NamespaceItem_8
        {
            .m_Node = {
                .m_Next = NamespaceItem_9,
                .m_Name = "m_OnBackColor",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__2,
            .m_Offset = CLASS_PROPERTY(ZENVIRONMENT, m_OnBackColor)
        };

        static RTP::ZDataProperty<ZCOLOR> NamespaceItem_7
        {
            .m_Node = {
                .m_Next = NamespaceItem_8,
                .m_Name = "m_OnColor",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__2,
            .m_Offset = CLASS_PROPERTY(ZENVIRONMENT, m_OnColor)
        };

        static RTP::ZVirtualProperty<bool> NamespaceItem_6
        {
            .m_Node = {
                .m_Next = NamespaceItem_7,
                .m_Name = "FogEnable",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZENVIRONMENT::GetFogEnable,
            .m_Set = &ZENVIRONMENT::SetFogEnable
        };

        static RTP::ZVirtualProperty<bool> NamespaceItem_5
        {
            .m_Node = {
                .m_Next = NamespaceItem_6,
                .m_Name = "CharacterShadow",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZENVIRONMENT::GetCharacterShadow,
            .m_Set = &ZENVIRONMENT::SetCharacterShadow
        };

        static RTP::ZVirtualProperty<bool> NamespaceItem_4
        {
            .m_Node = {
                .m_Next = NamespaceItem_5,
                .m_Name = "CharacterLight",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZENVIRONMENT::GetCharacterLight,
            .m_Set = &ZENVIRONMENT::SetCharacterLight
        };

        static RTP::ZVirtualProperty<bool> NamespaceItem_3
        {
            .m_Node = {
                .m_Next = NamespaceItem_4,
                .m_Name = "AmbientEnable",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__1,
            .m_Get = &ZENVIRONMENT::GetAmbientEnable,
            .m_Set = &ZENVIRONMENT::SetAmbientEnable
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZENVIRONMENT,
        ZLIGHT,
        0x009728B8,
        "ZENVIRONMENT",
        0x0076A138,
        cProperties::NamespaceItem_3,
        0x00806860,
        0x00972868,
        0x0097286C
    );
#   pragma endregion
}
