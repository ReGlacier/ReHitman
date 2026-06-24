#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZAnimVariationHandle.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>

#include <BloodMoney/Game/ZHM3ClipParticleControl.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>
#include <BloodMoney/Game/Items/EHM3WeaponScope.h>
#include <BloodMoney/Game/Items/ESilencerType.h>
#include <BloodMoney/Game/Items/EHM3RecoilRandom.h>

namespace Hitman::BloodMoney
{
    class ZHM3ItemTemplateWeapon : public Glacier::ZItemTemplateWeapon
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
        virtual EHM3WeaponScope GetScope();
        virtual ESilencerType GetSilencerType();
        virtual float GetWeaponStrength();
        virtual int Get1stPersonAimId();
        virtual int Get1stPersonRecoilId();
        virtual const char* GetAnimNameActorReload(int);
        virtual const char* GetAnimNameActorChamber();
        virtual void* GetClipParticleControl(bool);

        //data (total size is 0x1A0, ZItemTemplateWeapon size is 0x15C)
        Glacier::ZAnimVariationHandle m_1stPersonAimId;
        Glacier::ZAnimVariationHandle m_1stPersonRecoilId;
        EHM3ItemType m_eHM3ItemType;
        ZHM3ClipParticleControl* m_pClipParticleControl;
        ZHM3ClipParticleControl* m_pFireShellParticleControl;
        EHM3RecoilRandom m_eHM3RecoilRandom;
        float m_fRecoilStrengthX;
        float m_fRecoilStrengthY;
        EHM3WeaponScope m_eScope;
        Glacier::ZRTString m_pAnimNameActorReload;
        Glacier::ZRTString m_pAnimNameActorChamber;
        Glacier::ZRTString m_pAnimName1stPersonAim;
        Glacier::ZRTString m_pAnimName1stPersonRecoil;
        float RecognitionDistance;
        float m_PelvisUpDown;
        float m_PelvisFrontBack;
        float m_PelvisLeftRight;
        ESilencerType m_eSilencerType;
    };
    RE_VERIFY_SIZE(ZHM3ItemTemplateWeapon, 0x1A0); // Verified
}