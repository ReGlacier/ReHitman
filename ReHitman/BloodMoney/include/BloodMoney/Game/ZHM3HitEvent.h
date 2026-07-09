#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Fysix/SHitInfo.h>
#include <BloodMoney/Game/ZHM3Damage.h>


namespace Hitman::BloodMoney
{
    struct ZHM3HitEvent
    {
        Glacier::ZVector3 fHitPos;
        Glacier::ZVector3 fHitDir;
        int iDamage;
        Glacier::ZREF rOwner;
        const char* szAnimName;
        int iBoneNum;
        int iDamageType;
        int iBigForce;
        Glacier::ZREF rWeapon;

        void InitFromHitInfo(Glacier::SHitInfo* pHitInfo)
        {
            fHitPos = pHitInfo->pColi->cp;
            fHitDir = pHitInfo->pColi->ln;
            rOwner = pHitInfo->rWeaponOwner;
            rWeapon = pHitInfo->rWeapon;
        }
        
        void InitFromDamage(ZHM3Damage* pDamage)
        {
            fHitPos = *pDamage->GetHitLocation();
            fHitDir = *pDamage->GetHitNormal();
            rOwner = pDamage->HitInfo.rWeaponOwner;
            rWeapon = pDamage->HitInfo.rWeapon;
            iDamageType = pDamage->m_eDamageType;
            // It was inlined in PC, so I may miss smth
        }

        void SetDamage(int value)
        {
            iDamage = value;
        }

        void SetDamageType(int iType)
        {
            iDamageType = iType;
        }

        void SetAnimName(const char* pzName)
        {
            szAnimName = pzName;
        }

        void SetBoneNumber(int iBone)
        {
            iBoneNum = iBone;
        }
    };
    RE_VERIFY_SIZE(ZHM3HitEvent, 0x34);
}