#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Items/ZItemWeapon.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
    using ZStackArrayItemWeaponMetaKeyEvents = Glacier::ZStackArrayInsert<5, int, float>;
    RE_VERIFY_SIZE(ZStackArrayItemWeaponMetaKeyEvents, 44);

    class ZHM3ItemWeapon : public Glacier::ZItemWeapon
    {
    public:
        //vftable
        virtual EHM3ItemType GetHM3ItemType();
        virtual void ChamberActivateAnimation();
        virtual void UseItemActivateAnimation();
        virtual const char* GetAnimNameActorReload();
        virtual const char* GetAnimNameActorChamber();
        virtual int GetClipBoneNr();
        virtual void InitializeMetaKeyEvents(Glacier::Animation::Header* );
        virtual void UpdateMetaKeyEvents();
        virtual void UpdateReloadShellAnim();
        virtual void UpdateReloadShellAnimMetaKeys();
        virtual void FireAnimCallback(Glacier::Animation::ActiveAnimation*, float, float, unsigned int);
        virtual bool IsDetectable(); //always true

        //data (total size is 0x15C, ZItemWeapon size is 0xDC)
        float m_fRecoilValues[5];
        float m_fRecoilDamping;
        float m_fRecoilShotImpact;
        float m_fRecoilMasterValue;
        Glacier::Animation::Header* m_pAnimReload;
        Glacier::Animation::Header* m_pAnimFire;
        Glacier::Animation::Header* m_pAnimChamber;
        Glacier::Animation::Header* m_pAnimUse;
        int32_t m_iClipBoneNr;
        int32_t m_nCurFrame;
        Glacier::ZVector3 m_vClipPosOld;
        bool m_bDualWeapon;
        bool m_bBeingReloaded;
        bool m_bSixShooter;
        RE_ADD_PADDING(1);
        Glacier::ZLNKOBJ* m_pGround;
        Glacier::ZREF m_rCurrentSound;
        Glacier::Animation::ActiveAnimation* m_pBoneAnim;
        ZStackArrayItemWeaponMetaKeyEvents m_MetaKeyEvents;
    };
    RE_VERIFY_SIZE(ZHM3ItemWeapon, 0x15C); // Verified
}