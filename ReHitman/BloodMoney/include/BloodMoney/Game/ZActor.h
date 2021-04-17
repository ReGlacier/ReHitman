#pragma once

#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Hitman::BloodMoney
{
    class ZActor : public Glacier::ZLNKWHANDS
    {
    public:
        // pub
        enum ACTORSTATE
        {
            STATE_0 = 0,
            STATE_1 = 1,
            STATE_2 = 2,
            STATE_3  =3
        };

        enum PATH_CANCEL_REASONS
        {
        };

        enum PATH_ERRORS
        {
        };

        // vftable
        virtual void SetActorRootTM(float const*,float const*);
        virtual void SetActorPosDir(float const*,float const*);
        virtual void GetActorRootTM(float *,float *);
        virtual void GetActorWorldPosition(float *);
        virtual void SetActorWorldPosition(float const*);
        virtual bool CanPlayAnimSegment(Glacier::Animation::Header*, float, float, float const*, float const*, bool, float, float);
        virtual bool IsUnconscious();
        virtual bool IsAwake();
        virtual bool IsSleeping();
        virtual void SetHero(uint32_t);
        virtual void SetActorState(ZActor::ACTORSTATE);
        virtual void OnViewEnter();
        virtual void OnViewLeave();
        virtual void* GetPathFinder4();
        virtual void EnablePathFinder(bool);
        virtual void SetStopDistance(float);
        virtual void SetEndDir(const Glacier::Vector3*);
        virtual const Glacier::Vector3* GetEndDir();
        virtual void MoveToPosition(float const*,float const*);
        virtual void SlideToPosition(float const*,float const*);
        virtual void SetPathNotify(float);
        virtual void SetPathNotifySyncToCycle(bool);
        virtual void SetEndAction(bool);
        virtual void RemovePath(uint8_t, ZActor::PATH_CANCEL_REASONS, bool);
        virtual void SendPathDone(bool);
        virtual void GetMoveFromPos(float *);
        virtual void* FindNextPathBlocker();
        virtual void SetShootIntoGround(bool);
        virtual void SetShootIntoGroundInFullbody(bool);
        virtual void SetStayInsidePath(bool);
        virtual void ShootIntoGround(float *, bool, bool);
        virtual void ShootIntoGroundRegularly(float *, bool, bool, float);
        virtual void SetMoveSpeedMultiplier(float);
        virtual void SetPathFinderEnabled(bool);
        virtual void OnPathRequest(uint32_t);
        virtual void OnPathFinished(uint32_t);
        virtual void OnPathCanceled(ZActor::PATH_CANCEL_REASONS);
        virtual void OnPathCanceledLockedDoor(uint32_t);
        virtual void OnPathError(ZActor::PATH_ERRORS);
        virtual void OnPathNotify(void);
        virtual void OnNewPath(void);
        virtual void OnSound(Glacier::REFTAB*);
        virtual void Die();
        virtual void DieByForce(float const*, float const*, float, uint32_t);
        virtual void Resurrect();
        virtual void Knockout();
        virtual void Revive();
        virtual void* GetKnockedOut();
        virtual void* GetActorList();
        virtual void SetDisableIdleAnimation(bool);
        virtual void* GetAudibleRoomList();
        virtual void LookAt(uint32_t);
        virtual bool IsSeeing();
        virtual void SetSeeing(bool);
        virtual bool IsVisible();
        virtual void SetVisible(bool);
        virtual void WantToLookAt(Glacier::ZGEOM*, bool);
        virtual void VerifyPlayerVisible();
        virtual void DeterminePathLookAt(Glacier::LocomotionInfo *, float *, float *, float *, float *);
        virtual bool CanLookAt(Glacier::ZGEOM*);
        virtual bool IsOnStairs();
        virtual void UpdatePositionOffScreen();
        virtual void UpdatePosition();
        virtual Glacier::Animation::Header* GetCurrentUBAnim(Glacier::ZItem*, Glacier::ZItem *, bool&);
        virtual void SetHoldWeaponUBAnim(Glacier::Animation::Header*);
        virtual uint32_t GetAnimOffset(bool const*);
        virtual uint32_t GetAnimOffset(Glacier::Animation::Header*);
        virtual Glacier::Animation::Header* GetAnimHeader(uint32_t);
        virtual void ShootIntoGroundCallback(Glacier::SExtendedImpactInfo*);

        // Total size is 0x900, base size is 0x3D0)
        Glacier::Vector3 m_ContactNormal;
        int m_field3DC;
        int m_field3E0;
        int m_field3E4;
        int m_field3E8;
        int m_field3EC;
        int m_field3F0;
        Glacier::ZHumanBoid* m_boid;
        int m_field3F8;
        int m_field3FC;
        int m_field400;
        int m_field404;
        int m_field408;
        int m_field40C;
        int m_field410;
        int m_field414;
        int m_field418;
        int m_field41C;
        int m_field420;
        int m_field424;
        int m_field428;
        int m_field42C;
        int m_field430;
        int m_field434;
        int m_field438;
        int m_field43C;
        int m_field440;
        int m_field444;
        int m_field448;
        int m_field44C;
        int m_field450;
        int m_field454;
        int m_field458;
        int m_field45C;
        int m_field460;
        int m_field464;
        int m_field468;
        int m_field46C;
        int m_field470;
        int m_field474;
        int m_field478;
        int m_field47C;
        bool m_MoveSetNr;
        bool m_PreferredState;
        bool m_LastPreferredState;
        bool field_B3;
        int m_field484;
        int m_field488;
        int m_field48C;
        uint16_t m_iQuePos;
        uint16_t m_iProgramQueSize;
        int m_field494;
        int m_field498;
        int m_field49C;
        int m_field4A0;
        int m_field4A4;
        int m_field4A8;
        int m_field4AC;
        int m_field4B0;
        int m_field4B4;
        int m_field4B8;
        int m_field4BC;
        int m_field4C0;
        int m_field4C4;
        int m_field4C8;
        int m_field4CC;
        int m_field4D0;
        int m_field4D4;
        int m_field4D8;
        int m_field4DC;
        int m_field4E0;
        int m_field4E4;
        int m_field4E8;
        int m_field4EC;
        int m_field4F0;
        int m_field4F4;
        int m_field4F8;
        int m_field4FC;
        int m_field500;
        int m_field504;
        int m_field508;
        int m_field50C;
        int m_field510;
        int m_field514;
        int m_field518;
        int m_field51C;
        int m_field520;
        int m_field524;
        int m_field528;
        int m_field52C;
        int m_field530;
        int m_field534;
        int m_field538;
        int m_field53C;
        int m_field540;
        int m_field544;
        int m_field548;
        int m_field54C;
        int m_field550;
        int m_field554;
        int m_field558;
        int m_field55C;
        int m_field560;
        int m_field564;
        int m_field568;
        int m_field56C;
        int m_pPathFinder;
        int m_field574;
        int m_field578;
        bool m_Action0;
        bool m_Action1;
        bool m_iEndAction;
        bool field_1AF;
        int m_iNextPathTarget;
        int m_field584;
        int m_field588;
        int m_iPathfinderDoorKeyMask;
        int m_field590;
        int m_field594;
        int m_field598;
        int m_field59C;
        int m_field5A0;
        int m_field5A4;
        int m_field5A8;
        int m_field5AC;
        float m_fMoveSpeedMultiplier;
        int m_field5B4;
        int m_field5B8;
        Glacier::Vector3 m_vEndDir;
        Glacier::Vector3 m_vEndPos;
        float m_fEndDist;
        float m_fPathNotify;
        float m_fPathNotifyNext;
        bool m_lPathDoneFlag;
        bool m_ePathCancelReason;
        bool m_actorState;
        bool field_213;
        int m_VariationSeed;
        int m_VariationMask;
        int m_field5EC;
        int m_field5F0;
        int m_field5F4;
        int m_field5F8;
        int m_field5FC;
        int m_field600;
        int m_field604;
        int m_field608;
        int m_field60C;
        int m_field610;
        int m_field614;
        int m_field618;
        int m_field61C;
        int m_field620;
        int m_field624;
        int m_field628;
        int m_field62C;
        int m_field630;
        int m_field634;
        int m_field638;
        int m_field63C;
        int m_field640;
        int m_field644;
        int m_field648;
        int m_field64C;
        int m_field650;
        int m_field654;
        int m_field658;
        int m_field65C;
        int m_field660;
        int m_field664;
        int m_field668;
        int m_field66C;
        int m_field670;
        int m_field674;
        int m_field678;
        int m_field67C;
        int m_field680;
        int m_field684;
        int m_field688;
        int m_field68C;
        int m_field690;
        int m_field694;
        int m_field698;
        int m_field69C;
        int m_field6A0;
        int m_field6A4;
        int m_field6A8;
        int m_field6AC;
        int m_field6B0;
        int m_field6B4;
        int m_field6B8;
        int m_field6BC;
        int m_field6C0;
        int m_field6C4;
        int m_field6C8;
        int m_field6CC;
        int m_field6D0;
        int m_field6D4;
        int m_field6D8;
        int m_field6DC;
        int m_field6E0;
        int m_field6E4;
        int m_field6E8;
        int m_field6EC;
        int m_field6F0;
        int m_field6F4;
        int m_field6F8;
        int m_field6FC;
        int m_field700;
        int m_field704;
        int m_field708;
        int m_field70C;
        int m_field710;
        int m_field714;
        int m_field718;
        int m_field71C;
        int m_field720;
        int m_field724;
        int m_field728;
        int m_field72C;
        int m_field730;
        int m_field734;
        int m_field738;
        int m_field73C;
        int m_field740;
        int m_field744;
        int m_field748;
        int m_field74C;
        int m_field750;
        int m_field754;
        int m_field758;
        int m_field75C;
        int m_field760;
        int m_field764;
        int m_field768;
        int m_field76C;
        int m_field770;
        int m_field774;
        int m_field778;
        int m_field77C;
        int m_field780;
        int m_field784;
        int m_field788;
        int m_field78C;
        int m_field790;
        int m_field794;
        int m_field798;
        int m_field79C;
        int m_field7A0;
        int m_field7A4;
        int m_field7A8;
        int m_field7AC;
        int m_field7B0;
        int m_field7B4;
        int m_field7B8;
        int m_field7BC;
        int m_field7C0;
        int m_field7C4;
        int m_field7C8;
        int m_field7CC;
        int m_field7D0;
        int m_field7D4;
        int m_field7D8;
        int m_field7DC;
        int m_field7E0;
        int m_field7E4;
        int m_field7E8;
        int m_field7EC;
        int m_field7F0;
        int m_field7F4;
        int m_field7F8;
        int m_field7FC;
        int m_field800;
        int m_field804;
        int m_field808;
        int m_field80C;
        int m_field810;
        int m_field814;
        int m_field818;
        int m_field81C;
        int m_field820;
        int m_field824;
        int m_field828;
        int m_field82C;
        int m_field830;
        int m_field834;
        int m_field838;
        int m_field83C;
        int m_field840;
        int m_field844;
        int m_field848;
        int m_field84C;
        int m_field850;
        int m_field854;
        int m_field858;
        int m_field85C;
        int m_field860;
        int m_field864;
        int m_field868;
        int m_field86C;
        int m_field870;
        int m_field874;
        int m_field878;
        int m_field87C;
        int m_field880;
        int m_field884;
        int m_field888;
        int m_field88C;
        int m_field890;
        int m_field894;
        int m_field898;
        int m_field89C;
        int m_field8A0;
        int m_field8A4;
        int m_field8A8;
        int m_field8AC;
        int m_field8B0;
        int m_field8B4;
        int m_field8B8;
        int m_field8BC;
        int m_field8C0;
        int m_field8C4;
        int m_field8C8;
        int m_field8CC;
        int m_field8D0;
        int m_field8D4;
        bool m_field8D8;
        bool m_field8D9;
        bool m_field8DA;
        bool m_field8DB;
        int m_field8DC;
        int m_lShootIntoGroundFlag;
        int m_field8E4;
        int m_animCurrent; // +0x8E8
        int m_field8EC;
        int m_field8F0;
        int m_field8F4;
        int m_field8F8;
        int m_field8FC;
    };
}