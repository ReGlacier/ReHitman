#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZMessageResolver.h>
#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>

namespace Hitman
{
    class ZHM3ItemBomb : public ZHM3ItemWeapon
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemBomb, 0x100450u);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // methods
        // PC 0x6506E0. Initializes the explosion state and the room-shatter list.
        ZHM3ItemBomb(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // static
        // PC 0x009B1A08 ("GetRoomShatterList") and 0x009B1A14 ("GetRoomShotActivateList").
        STATIC_CLASS_VAR(ZHM3ItemBomb, Glacier::ZMessageResolver, m_msgGetRoomShatterList);
        STATIC_CLASS_VAR(ZHM3ItemBomb, Glacier::ZMessageResolver, m_msgGetRoomShotActivateList);

        // api
        void Explode();
        // PC 0x64CE00. Spawns the template's effect group at the bomb's root transform.
        void ActivateEffect();

        // data (total size is 0x2A0, base size is 0x15C)
        bool m_bExploded;
        bool m_bExploding;
        bool m_bTimerActivated;
        RE_ADD_PADDING(1);
        uint32_t m_iNumberOfTargets;
        Glacier::ZBaseGeom* m_pTargets[30];
        uint32_t m_iTargetCheck;
        Glacier::TIMETYPE m_ttTimeToExplode;
        uint32_t m_iNumberOfLines;
        Glacier::REFTAB32 m_Lines;
        Glacier::ZGEOM* m_pPlaceBombActionGeom;
        uint32_t m_nTopEvCamId;
        Glacier::TIMETYPE m_ttTopViewCamStartTime;

    };
    RE_VERIFY_SIZE(ZHM3ItemBomb, 0x2A0); // Verified
}