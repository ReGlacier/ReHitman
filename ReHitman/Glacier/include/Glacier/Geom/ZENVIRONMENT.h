#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/RTP/PropertyTypes.h>


namespace Glacier
{
    class ZENVIRONMENT : public ZLIGHT
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZENVIRONMENT, 0x80000Du);

        // static
        static constexpr uint32_t ZLE_AMBIENT_LIGHT = 0x1000u;
        static constexpr uint32_t ZLE_FOG = 0x2000u;
        static constexpr uint32_t ZLE_CHARACTER_LIGHT = 0x4000;
        static constexpr uint32_t ZLE_CHARACTER_SHADOW = 0x8000;

        // vtbl
        ~ZENVIRONMENT() override;

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void ClassInit() override;
        void CopyData(const ZGEOM* Source) override;

        // ZLIGHT
        void SetMultiplier(float fValue) override;
        void SetDiffuseColor(uint32_t lColor) override;

        // methods
        ZENVIRONMENT(const char* psName, ZBaseGeom* pBaseGeom);

        void ToggleColor(bool bOnOff);
        void SetDiffuseColor(uint32_t dwColor, uint32_t dwBackColor);

#       pragma region " --- RTTI Methods --- "
        void GetAmbientEnable(bool& bEnable);
        void SetAmbientEnable(const bool& bEnable);
        void GetCharacterLight(bool& bLight);
        void SetCharacterLight(const bool& bLight);
        void GetCharacterShadow(bool& bShadow);
        void SetCharacterShadow(const bool& bShadow);
        void GetFogEnable(bool& bEnable);
        void SetFogEnable(const bool& bEnable);
#       pragma endregion

        // members
        ZENVIRONMENT* m_pCharacterLight;
        ZENVIRONMENT* m_pCharacterShadow;
        ZCOLOR m_OnColor;
        ZCOLOR m_OnBackColor;
        ZCOLOR m_OffColor;
        ZCOLOR m_OffBackColor;
    };
    RE_VERIFY_SIZE(ZENVIRONMENT, 0x38); // Verified PC alloc
}
