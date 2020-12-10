#pragma once

#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplateWeapon : Glacier::ZItemTemplateWeapon
    {
    public:
        //vftable
        virtual EHM3ItemType GetHM3ItemType();
        virtual float GetRecoilRandom();
        virtual float GetRecoilStrengthX();
        virtual float GetRecoilStrengthY();
        virtual void* GetLaserIndicator();
        virtual void* GetRedDot();
        virtual bool CanPackIntoSuitcase();
        virtual void* GetScope();
        virtual int GetSilencerType();
        virtual float GetWeaponStrength();
        virtual int Get1stPersonAimId();
        virtual int Get1stPersonRecoilId();
        virtual const char* GetAnimNameActorReload(int);
        virtual const char* GetAnimNameActorChamber();
        virtual void* GetClipParticleControl(bool);

        //data (total size is 0x1A0, ZItemTemplateWeapon size is 0x15C)
        int m_field15C;
        int m_field160;
        int m_field164;
        int m_field168;
        int m_field16C;
        int m_field170;
        int m_field174;
        int m_field178;
        int m_field17C;
        int m_field180;
        int m_field184;
        int m_field188;
        int m_field18C;
        int m_field190;
        int m_field194;
        int m_field198;
        int m_field19C;
    };
}