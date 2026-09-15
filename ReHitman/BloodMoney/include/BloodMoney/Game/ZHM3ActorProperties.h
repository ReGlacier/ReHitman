#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>


namespace Hitman::BloodMoney
{
    /**
     * I guess, this class is not used by game, but available in RTTI and looks incomplete
     */
    class ZHM3ActorProperties : public Glacier::ZSTDOBJ
    {
    public:
        // vtbl
        virtual Glacier::REFTAB* GetKnownHitmanAses();

        // data
        Glacier::REFTAB* m_prtKnownHitmanAses; // 0x10
        float m_fRecognizeSpeedKnown; // 0x14
        int m_iForgetTime; // 0x18
        float m_fInspectionDistMin; // 0x1C
        float m_fInspectionDistMax; // 0x20
    }; // Verified size 0x24
    RE_VERIFY_SIZE(ZHM3ActorProperties, 0x24); // Verified
    RE_VERIFY_OFFSET(ZHM3ActorProperties, m_prtKnownHitmanAses, 0x10);
}