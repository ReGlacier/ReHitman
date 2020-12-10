#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Items/ZItemWeapon.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman::BloodMoney
{
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
        int m_fieldDC;
        int m_fieldE0;
        int m_fieldE4;
        int m_fieldE8;
        int m_fieldEC;
        int m_fieldF0;
        int m_fieldF4;
        int m_fieldF8;
        int m_fieldFC;
        int m_field100;
        int m_field104;
        int m_field108;
        int m_clipBoneNr;
        int m_field110;
        int m_field114;
        int m_field118;
        int m_field11C;
        int m_field120;
        Glacier::ZLNKOBJ* m_field124;
        int m_field128;
        int m_field12C;
        int m_field130;
        int m_field134;
        int m_field138;
        int m_field13C;
        int m_field140;
        int m_field144;
        int m_field148;
        int m_field14C;
        int m_field150;
        int m_field154;
        int m_field158;
    };

}