#pragma once
#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/RTP/PropertyTypes.h>

namespace Glacier
{
    class ZEnvSampler : public ZSTDOBJ
    {
    public:
        enum UPDATE_FREQUENCY : uint32_t { UF_ONCE = 0, UF_SLOW = 1, UF_FAST = 2 };
        DECLARE_GEOM_CLASS(ZEnvSampler, 0x20010Fu);
        ~ZEnvSampler() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t&, uint32_t&) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void ClassInit() override;
        void CopyData(const ZGEOM*) override;
        ZEnvSampler(const char*, ZBaseGeom*);
        uint32_t m_lIdentifier;
        float m_fCameraFar, m_fFogNear, m_fFogFar;
        ZCOLOR m_lFogColor;
        UPDATE_FREQUENCY m_eUpdateFrequency;
        bool m_bCurrentRoomOnly;
        uint8_t m_PAD_29[7];
    };
    RE_VERIFY_SIZE(ZEnvSampler, 0x30);
}
