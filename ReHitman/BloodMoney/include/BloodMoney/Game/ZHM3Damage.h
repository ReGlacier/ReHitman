#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Physics/SHitInfo.h>
#include <Glacier/Physics/COLI.h>


namespace Hitman::BloodMoney
{
    enum EDamageType 
    {
        UNKNOWN = 0,
        PROJECTILE = 1,
        UNARMED = 2,
        CHLOROFORM = 3,
        STRANGULATE = 4,
        EDGED = 5,
        CUTTHROAT = 6,
        EXPLOSIVE = 7,
        CLOSECOMBAT = 8,
        BLUNT = 9,
        LARGEBLUNT = 10,
        PUSHED = 11,
        FALL = 12,
        POISON = 13,
        ANASTETIC = 14,
        LAXATIVE_THISISNOTUSED = 15,
        STRANGLE_IN_ELEVATOR = 16,
        APHRODICIAC = 17,
    };

    struct ZHM3Damage
    {
        Glacier::THitInfo HitInfo;
        Glacier::COLI m_Coli;
        EDamageType m_eDamageType;
        bool m_bBigForce;
        float m_fBaseDamage;
        uint32_t m_BodyPart;

        Glacier::ZVector3* GetHitLocation() { return &HitInfo.pColi->cp; }
        Glacier::ZVector3* GetHitNormal() { return &HitInfo.pColi->ln; }
    };
    RE_VERIFY_SIZE(ZHM3Damage, 0x90);
}