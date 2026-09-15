#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    enum _eDirection
    {
        eDirection_LEFT = 0,
        eDirection_RIGHT = 1,
        eDirection_UP = 2,
        eDirection_DOWN = 3,
        eDirection_TOWARDS = 4,
        eDirection_AWAY = 5,
        eDirection_FLANKING = 6,
        eDirection_ANY = 7,
    };
    using eDirection = _eDirection;

    enum _eAttackStyle
    {
        eAttackStyle_RETREAT = 0,
        eAttackStyle_DEFENSIVE = 1,
        eAttackStyle_NORMAL = 2,
        eAttackStyle_AGGRESIVE = 3,
    };
    using eAttackStyle = _eAttackStyle;

    enum _eAudioEvent
    {
        eAudioEvent_NONE = 0,
        eAudioEvent_SHOT = 1,
        eAudioEvent_SHOT_SILENCED1 = 2,
        eAudioEvent_SHOT_SILENCED2 = 3,
        eAudioEvent_PAIN = 4,
        eAudioEvent_DIE = 5,
        eAudioEvent_STRANGULATE = 6,
        eAudioEvent_CUTTHROAT = 7,
        eAudioEvent_FOOTSTEP = 8,
        eAudioEvent_WEAPON_DRAWN = 9,
        eAudioEvent_WEAPON_RELOAD = 10,
        eAudioEvent_BULLET_IMPACT = 11,
        eAudioEvent_ALARM_LOCAL = 12,
        eAudioEvent_ALARM_GLOBAL = 13,
        eAudioEvent_EXPLOSION = 14,
        eAudioEvent_HELPREQUEST = 15,
        eAudioEvent_DOOR = 16,
        eAudioEvent_DISTRACTION = 17,
        eAudioEvent_MOANING = 18,
        eAudioEvent_HITMAN_SUSPICIOUS = 19,
        eAudioEvent_HITMAN_THREATENING = 20,
        eAudioEvent_EVENTS_END = 21,
    };
    using eAudioEvent = _eAudioEvent;

    enum _eAudioType
    {
        eAudioType_PAIN = 0,
        eAudioType_DIE = 1,
        eAudioType_CUTTHROAT = 2,
        eAudioType_IDLE = 3,
        eAudioType_PANIC = 4,
        eAudioType_INTERESTINGSOUND = 5,
        eAudioType_STRANGESOUND = 6,
        eAudioType_SUSPICIOUSSOUND = 7,
        eAudioType_THREATENINGSOUND = 8,
        eAudioType_STANDDOWNINTERESTING = 9,
        eAudioType_STANDDOWNSTRANGE = 10,
        eAudioType_STANDDOWNSUSPICIOUS = 11,
        eAudioType_BODYLOSTCLOTHES = 12,
        eAudioType_BODYMOVED = 13,
        eAudioType_FINDWEAPON = 14,
        eAudioType_HMNEARBODY = 15,
        eAudioType_SEEBLOOD = 16,
        eAudioType_SEEBODY = 17,
        eAudioType_SEECORPSE = 18,
        eAudioType_HMTARGETED = 19,
        eAudioType_HMPICKLOCK = 20,
        eAudioType_HMSNEAKBY = 21,
        eAudioType_HMWINDOWWALL = 22,
        eAudioType_FRISKSTART = 23,
        eAudioType_FRISKWEAPONFOUND = 24,
        eAudioType_FRISKCONFISCATE = 25,
        eAudioType_FRISKOK = 26,
        eAudioType_HMATTACKING = 27,
        eAudioType_HMSWAPINGCLOTHES = 28,
        eAudioType_HMDRAGMALEBODY = 29,
        eAudioType_HMDRAGFEMALEBODY = 30,
        eAudioType_HMHASGUN = 31,
        eAudioType_RUNFORBACKUP = 32,
        eAudioType_STRANGLE = 33,
        eAudioType_PUSHED = 34,
        eAudioType_TRESPASS3 = 35,
        eAudioType_TRESPASS2 = 36,
        eAudioType_TRESPASS1 = 37,
        eAudioType_HMHASNOGUN = 38,
        eAudioType_LIGHTPLAYCANSEE = 39,
        eAudioType_LIGHTPLAYCANTSEE = 40,
        eAudioType_LIGHTPLAYWARNING1 = 41,
        eAudioType_LIGHTPLAYWARNING2 = 42,
        eAudioType_LIGHTPLAYWARNING3 = 43,
        eAudioType_HUMANSHIELDSPOTSHIELD = 44,
        eAudioType_HUMANSHIELDHELDASSHIELD = 45,
        eAudioType_MOANING = 46,
        eAudioType_SUFFERING = 47,
        eAudioType_FRISKHMRUNSTHROUGH = 48,
        eAudioType_FRISKHMWITHGUN1 = 49,
        eAudioType_FRISKHMWITHGUN2 = 50,
        eAudioType_FRISKHMWITHGUN3 = 51,
        eAudioType_FRISKHMWONTSTANDSTILL1 = 52,
        eAudioType_FRISKHMWONTSTANDSTILL2 = 53,
        eAudioType_FRISKHMWONTSTANDSTILL3 = 54,
        eAudioType_FRISKHMWITHGUNAGGRESSIVE1 = 55,
        eAudioType_FRISKHMWITHGUNAGGRESSIVE2 = 56,
        eAudioType_SEECORPSEFEMALE = 57,
        eAudioType_CHATTER = 58,
        eAudioType_NOTORIETYRECOGNITION = 59,
        eAudioType_NOTORIETYATTACK = 60,
        eAudioType_LIFTING = 61,
        eAudioType_BODYBAGGIN = 62,
        eAudioType_GUARDFINDBROKENCAMERA = 63,
        eAudioType_GUARDFINDSUITCASE = 64,
        eAudioType_HMSTEALSITEM = 65,
        eAudioType_PICKLOCKWARNING1 = 66,
        eAudioType_PICKLOCKWARNING2 = 67,
        eAudioType_LEAVEDOOR = 68,
        eAudioType_PUSHSHORT = 69,
    };
    using eAudioType = _eAudioType;

    enum _eEventCameraPosition
    {
        eEventCameraPosition_DefaultPosition = 0,
        eEventCameraPosition_Right = 1,
        eEventCameraPosition_Bottom = 2,
    };
    using eEventCameraPosition = _eEventCameraPosition;

    enum _eEventCameraType
    {
        eEventCameraType_None = 0,
        eEventCameraType_DeadBody = 1,
        eEventCameraType_Sniper = 2,
        eEventCameraType_TopView = 3,
        eEventCameraType_FrontView = 4,
        eEventCameraType_CamBoneAnim = 5,
        eEventCameraType_AttachToGeom = 6,
        eEventCameraType_AttachToGeomLookAtGeom = 7,
        eEventCameraType_BombTrigger = 8,
        eEventCameraType_ActorFall = 9,
        eEventCameraType_SecurityCamera = 10,
        eEventCameraType_SideView = 11,
    };
    using eEventCameraType = _eEventCameraType;

    enum _eGQCTaskType
    {
        eGQCTaskType_None = 0,
        eGQCTaskType_PickUpWeapon = 1,
        eGQCTaskType_PickUpDeadBody = 2,
        eGQCTaskType_PickUpSuitcase = 3,
    };
    using eGQCTaskType = _eGQCTaskType;

    enum _eMeta
    {
        eMeta_PICKUP = 0,
        eMeta_START = 1,
        eMeta_END = 2,
        eMeta_STARTRIGIDBODY = 3,
        eMeta_ENDRIGIDBODY = 4,
        eMeta_CONTACT = 5,
        eMeta_JUMP_CONTACT = 6,
        eMeta_CAMERA_ON = 7,
        eMeta_CAMERA_OFF = 8,
        eMeta_SOUNDSTART = 9,
        eMeta_SOUNDEND = 10,
        eMeta_WEAPONCLIPHIDE = 11,
        eMeta_WEAPONCLIPSHOW = 12,
        eMeta_WEAPONCLIPRELEASE = 13,
        eMeta_ALIGNEND = 14,
        eMeta_IMPACT_BLOOD_CLOUD = 15,
        eMeta_LEFT_HAND = 16,
        eMeta_RIGHT_HAND = 17,
        eMeta_PUT_DOWN = 18,
        eMeta_SHOOT_START = 19,
    };
    using eMeta = _eMeta;

    enum _eScriptPathType
    {
        eScriptPathType_LOOPING = 0,
        eScriptPathType_PINGPONG = 1,
        eScriptPathType_RANDOM = 2,
        eScriptPathType_ONEWAY = 3,
        eScriptPathType_ERROR = 4,
    };
    using eScriptPathType = _eScriptPathType;

    enum _eWeaponType
    {
        eWeaponType_NoWeapon = 0,
        eWeaponType_CloseCombat = 1,
        eWeaponType_Gun = 2,
        eWeaponType_Smg = 3,
        eWeaponType_Rifle = 4,
        eWeaponType_OtherWeapon = 5,
    };
    using eWeaponType = _eWeaponType;

    using _v3 = ZVector3; // Just for SI naming compatibility
    using v3 = _v3; // Just for SI naming compatibility

    struct _sCover
    {
        v3 position;
        v3 facing;
        eDirection movedirection;
        float distance;
        int indx;
    };
    using sCover = _sCover;

    struct _sGQCTask
    {
        eGQCTaskType type;
        ZREF rtarget;
    };
    using sGQCTask = _sGQCTask;

    struct _ZSC_EVENT
    {
        ZMSGID zmsg;
        void* pData;
    };
    using ZSC_EVENT = _ZSC_EVENT;

    using anim = uint16_t;
    using STATEREF = const void**;

    enum _eMoveSet
    {
        eMoveSet_NORMAL = 0,
        eMoveSet_SLOW = 1,
        eMoveSet_DRUNK = 2,
        eMoveSet_DOG = 3,
        eMoveSet_RAT = 4,
        eMoveSet_DRAG = 5,
        eMoveSet_JOGG = 6,
        eMoveSet_DRILL = 7,
        eMoveSet_TOURGUIDE = 8,
        eMoveSet_SNEAK = 9,
        eMoveSet_CRATE = 10,
        eMoveSet_BELLYDANCE = 11,
        eMoveSet_M11ASSASSINCARTWHEELLEFT = 12,
        eMoveSet_M11ASSASSINCARTWHEELRIGHT = 13,
        eMoveSet_M11ASSASSINFASTRUN = 14,
        eMoveSet_eMoveSetEnd = 15,
    };
    using eMoveSet = _eMoveSet;
}
