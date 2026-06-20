#pragma once

#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/Game/LevelControls/ZHM3LevelControl.h>
#include <BloodMoney/Game/ZHM3BriefingControl.h>
#include <BloodMoney/Game/UI/ZOSD.h>

#include <Glacier/ZGameData.h>
#include <Glacier/ZSTL/ZArray.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZBaseConRout.h>

#define DECLARE_UNKNOWN_TYPE_PTR(name) using name = std::uintptr_t;
#define VERIFY_FIELD_POS(cls, fld, expected) static_assert(offsetof(cls, fld) == (expected), "Bad offset of " #cls "::" #fld);

namespace Glacier
{
    class ZWINDOW;
    class ZPlayer;

    class ZGameDataFactoryBase
    {
    public:
        virtual void CreateGameData();
        virtual void DestroyGameData();
    };

    class ZGameDataFactory : ZGameDataFactoryBase
    {
    };

    class ZGameStats
    {
    public:
        virtual ~ZGameStats();

    public: // members
        short m_iStats_CurrentShotCount; // +0x4 [Fiberwire and all attacks counted here]
        short m_Padding[1];              // +0x6
        float m_iStats_LastShotTime;     // +0x8
    };

    VERIFY_FIELD_POS(ZGameStats, m_iStats_CurrentShotCount, 0x4);
    VERIFY_FIELD_POS(ZGameStats, m_iStats_LastShotTime, 0x8);

    template <typename T, size_t N>
    struct ZStaticVector
    {
        uint32_t m_iSize;
        T m_Data[N];

        T& operator[](size_t i) { return m_Data[i]; }
    };
}

namespace Hitman::BloodMoney
{
    DECLARE_UNKNOWN_TYPE_PTR(N00001B3F);
    DECLARE_UNKNOWN_TYPE_PTR(N000033C1);
    DECLARE_UNKNOWN_TYPE_PTR(N0000163D);
    DECLARE_UNKNOWN_TYPE_PTR(CGlobalCom);
    DECLARE_UNKNOWN_TYPE_PTR(ZHM3CameraEventCameraClass);

    class ZHM3DialogControl;
    class ZHM3MenuElements;
    class ZHM3Hitman3;
    class ZGui;
    class CIngameMap;
    class ZHM3WeaponUpgradeControl;
    class ZHM3CameraClass;
    class ZBoidSystem;

    class ZHM3GameStats : public Glacier::ZGameStats
    {
    public:
        int m_iShotsHit;  // +0xC
        int m_iShotCount; // +0x10
    };

    VERIFY_FIELD_POS(ZHM3GameStats, m_iShotsHit, 0xC);
    VERIFY_FIELD_POS(ZHM3GameStats, m_iShotCount, 0x10);

    enum EBriefingDetailType : int {
        eBriefingTypeNoType = 0,
        eBriefingTypeTarget = 1,
        eBriefingTypeRetrieve = 2,
        eBriefingTypeEscape = 3,
        eBriefingTypeDispose = 4,
        eBriefingTypeProtect = 5,
        eBriefingTypeOptional = 6
    };

    class ZLevelLinking
    {
    public:
        EBriefingDetailType m_eBriefingDetailType; //0x0000
        void* m_pCom; //0x0004
        uint8_t m_Profile[24216]; //0x0008
    }; //Size: 0x5EA0

    struct sDifficultySettings {
        float fstanddowntime;
        float ffollowtrailtime;
        float fhunttargettracktime;
        float fseethroughdisguisedistance;
        float fseethroughdisguisespeed;
        float fforgetdisguisespeed;
        float frunsuspiciousrange;
        float faccuracymodifier;
        float fburstlength;
        float frofmultiplier;
        float faimspeed;
        float fdrawspeedmultiplier;
        float fshothearingrange;
        float fscreamhearingrange;
        float fcheckfootstepsbehindrange;
        float factorrelaxedvisionrange;
        float factorrelaxedfov;
        float factoralertvisionrange;
        float factoralertfov;
        float ffallingdamagemultiplier;
        float fusecoverpercentage;
        float fbleedtodeathtime;
        float fciviliansattackhmpercentage;
        float fcivilianhidetime;
        float flockpicktimemultiplier;
        float ffriskintrusiondecaytime;
        float fsmallweaponvisiblemultiplierciv;
        float fsmallweaponvisiblemultipliergrd;
        int ihitmanhitpoints;
        int iguardhitpoints;
        int icivilianhitpoints;
        int itargethitpoints;
        bool bautoaim;
        bool bheadshotsagainsthitman;
        bool bpad[2];
        float fmoneymultiplier;
        float fmaxmoneymultiplier;
        float fcleanupprice;
        float finnocentbribeprice;
        float fpolicebribeprice;
        float fnewidentityprice;
        float fweaponupgradeprice;
        float fweaponretrieveprice;
    };

    static_assert(sizeof(sDifficultySettings) == 164, "Bad size of sDifficultySettings");

    enum EDifficulty : uint32_t {
        eeROOKIE = 0,
        eNORMAL = 1,
        eEXPERT = 2,
        ePROFESSIONAL = 3,

        eMAX_VALUE = 4
    };

    class ZDifficultySettings
    {
    public:
        sDifficultySettings aSettings[EDifficulty::eMAX_VALUE];
    };

    struct sPricesStruct {
        int iObjectivePrices[13][6];
        int iTierPrices[5];
        int iTierUpgradePrice[5];
        int iExtraBriefing;
        int iAdditionalInfo;
        int iPlaceClothbundle;
        int iPlaceHandgun;
        int iPlaceSilencedHandgun;
        int iPlaceSMG;
        int iPlaceExtraSyringes;
        int iDisablingAlarms;
        int iGuardsNotFrisking;
        int iRetrieveWeapons;
        int iReplacementSuit;
        int iInnocentDamageControl;
        int iPoliceDamageControl;
        int iRetrieveVideotapes;
        int iBribeInnocent;
        int iBribePolice;
        int iNewIdentity;
    };

    // Size: 0x1A4 (420 bytes)
    struct ZMoneySystem {
        sPricesStruct m_sPrices;
    };

    struct sNotorietyModifiers {
        int iCaughtOnTapeDisguised;
        int iCaughtOnTapeNotDisguised;
        int iWitness1;
        int iWitness2;
        int iWitness3;
        int iWitness4;
        int iWitnessx;
        int iMaxWitnessesNormal;
        int iMaxWitnessesExpert;
        int iMaxWitnessesProfessional;
        int iBribeInnocent;
        int iBribePolice;
        int iNewIdentity;
    };

    struct ZNotorietySystem
    {
        sNotorietyModifiers m_sNotorietyModifiers;
    };

    static_assert(sizeof(ZNotorietySystem) == 52, "Bad size of ZNotorietySystem");

    struct ZRatingSystem
    {
        // There are nothing, all stored in static vars
    };

    enum EFriskOp : int
    {
        eeFriskOp_Attack = 0,
        eFriskOp_DefendAreaNormal = 1,
        eFriskOp_DefendAreaAggressive = 2,
        eFriskOp_TakeWeapons = 3
    };

    struct ZBloodTrail : public Glacier::ZSTDOBJ 
    {
        int m_iTrailIndex;
        int m_iElementNr;
        bool m_bIsRegistered;
    };

    class ZTrailControl {
    public:
        virtual ~ZTrailControl();

        struct ZBloodTrail* m_pTrails[5][5];
        bool m_bActiveElements[5][5];
        int m_bDecalIndex[5][5];
        bool m_bActiveTrails[5];
        short m_NumElements;
        int m_currentTrail;
        int m_currentElement;
        int m_currentReadTrail;
        int m_currentReadElement;
    };

    static_assert(sizeof(ZTrailControl) == 0x100, "Bad size of ZTrailControl");

    enum EUpgradeType : int {
        UT_Dummy = 0, UT_AmmoACP = 1, UT_AmmoArmorPiercing = 2, UT_Ammo127mm = 3, UT_AmmoMagnum = 4,
        UT_AmmoFlechetteSlugs = 5, UT_AmmoGaugesSlugs = 6, UT_AmmoLowVelocity = 7, UT_Magazine = 8,
        UT_Silencer1 = 9, UT_Silencer2 = 10, UT_LaserSight = 11, UT_DualAction = 12, UT_DualActionAuto = 13,
        UT_DoubleCapMag = 14, UT_FullAuto = 15, UT_ReloadBoost = 16, UT_BeltFeeding = 17, UT_BiPod = 18,
        UT_ScopeType1 = 19, UT_ScopeType2 = 20, UT_ScopeType3 = 21, UT_NightVision = 22, UT_Lightweight = 23,
        UT_DefaultAmmo = 24, UT_DefaultNoScope = 25, UT_DefaultBarrel = 26, UT_DefaultMagazine = 27,
        UT_RailMount = 28, UT_CarbonBarrel = 29, UT_Buttstock = 30, UT_Suitcase = 31, UT_DefaultButtStock = 32,
        UT_DefaultGrip = 33, UT_DefaultHandguard = 34, UT_DefaultHandle = 35, UT_DefaultSight = 36,
        UT_BoltAction = 37, UT_RedDotSight = 38, UT_ShortBarrel = 39, UT_PistolGrip = 40, UT_HandGuard = 41,
        UT_RapidFire = 42, UT_LongSlide = 43, UT_ClipX2 = 44, UT_ClipX3 = 45, UT_ClipX4 = 46,
        UT_NumUpgradeTypes = 47
    };

    struct ZWeaponUpgradeUtils {
        Glacier::REFTAB* m_pUpgradeList;
        uint32_t m_ALIGN4;
        unsigned long long m_lAvailableUpgradesMask;
        int m_lLastUpgrade;
        int m_lCurrentUpgrade;
        EHM3ItemType m_eCurrentSelectedItem;
        EUpgradeType m_eCurrentUpgrade;
    };

    static_assert(sizeof(ZWeaponUpgradeUtils) == 0x20, "Bad size of ZWeaponUpgradeUtils");

    struct SClothInfo {
        unsigned int rHitmanAs;
        float fDisguiseQuality;
        bool bDisguiseBlown;
        bool m_bPad[3];
    };

    struct SInspector {
        unsigned int rSeerActor;
        float fQuality;
        struct ZHM3ActorProperties* rt_pActorProperty;
        unsigned char iClothIdx;

        union {
            unsigned char mask;
            struct {
                unsigned char bInvestigating : 1;
                unsigned char bKnownCloth : 1;
                unsigned char bNotorietyBlowCover : 1;
            };
        };
        bool m_bPad[2];
    };

    struct ZClothTracker : public Glacier::ZBaseConRout
    {
        Glacier::ZArray<SClothInfo> m_Clothes;
        Glacier::ZArray<SInspector> m_Inspectors;

        float m_fInspectTime;
        float m_fDiffSpeedMult;
        float m_fDiffForgetMult;
        float m_fDiffSeeThroughDisguiseDistance;

        int m_iNotoriety;
        float m_fNotorietySpeedMult;
        float m_fNotorietyDistMult;
        int m_iCurrInspectCloth;
    };

    static_assert(sizeof(ZClothTracker) == 0x88, "Bad size of ZClothTracker");

    class ZHM3GameData final : public Glacier::ZGameData
    {
    public:
        // >>>>>>>>>>> ZGameData <<<<<<<<<<<
        // DronCode: Need to move that members to Glacier::ZGameData but it refs to HM3 stuff, need reverse base classes, will do it later
        ZBoidSystem* m_BoidSystem; //0x0004
        Glacier::ZStaticVector<ZHM3Actor*, 512> m_ActorsPool;
        Glacier::ZLIST* m_AllActorsAndPlayerList; //0x080C
        Glacier::ZStaticVector<Glacier::ZGEOM*, 128> m_ItemsPool;
        bool m_bDisableDust;
        RE_ADD_PADDING(3);
        Glacier::REFTAB* m_ParticleTemplatesList; //0x0A18
        ZHM3MenuElements* m_MenuElements; //0x0A1C
        Glacier::ZPlayer* m_apPlayers[4];
        ZHM3GameStats* m_GameStats; //0x0A30
        int m_pSoundDef_OSD; //0x0A34 ZSDOwner* m_pSOund
        int m_pSoundDef_Effects; //0x0A38
        bool m_bGameJustLoaded;
        bool m_bPad[3];

        // >>>>>>>>>>> ZHM3GameData <<<<<<<<<<<
        ZHM3Hitman3* m_Hitman3; //0x0A40
        Glacier::ZREF m_rPlayer; //0x0A44
        ZHM3LevelControl* m_LevelControl; //0x0A48
        ZOSD* m_OSD; //0x0A4C
        ZGui* m_Gui; //0x0A50
        CIngameMap* m_IngameMap; //0x0A54
        ZLevelLinking m_LevelLinking;
        ZHM3BriefingControl m_BriefingControl;
        Glacier::ZWINDOW* m_pZWindow;
        ZHM3WeaponUpgradeControl* m_pWeaponUgradeControl; //0x6970
        ZHM3CameraEventCameraClass* m_pEventCamera; //0x6974
        ZHM3CameraClass* m_pMainCamera; //0x6978
        int m_pActorMoveSets;
        ZDifficultySettings* m_pDifficultySettings;
        ZMoneySystem* m_pMoneySystem;
        ZNotorietySystem* m_pNororietySystem;
        ZRatingSystem* m_pRatingSystem;
        uint32_t m_rActorCommunication;
        uint32_t m_rFightController;
        uint32_t m_rCoverList;
        ZClothTracker* m_pClothTracker; //0x699C [always nullptr, lmao]
        uint32_t m_rActorFriskingHM;
        EFriskOp m_eFriskOperation; //0x69A0
        ZHM3DialogControl* m_DialogControl; //0x69A8
        Glacier::REFTAB m_Elevators;
        ZWeaponUpgradeUtils* m_pWeaponUpgradeUtils;
        Glacier::REFTAB m_FriskGuards;
        Glacier::REFTAB m_ItemTemplates;
        float m_fTension;
        float m_fTensionDelta;
        Glacier::ZGROUP* m_pBodyBagGroup;
        unsigned int* m_pTotalWeaponPrims;
        int m_lTotalWeaponPrimsCount;
        unsigned int* m_pUsedWeaponPrims;
        int m_lUsedWeaponPrimsCount;
        unsigned int* m_pBigWeaponPrims;
        int m_lBigWeaponPrimsCount;
        ZTrailControl* m_pTrailControl;
        int m_iPFBodyBagBoxTypeId;
        Glacier::REFTAB m_WantedPosts;
        bool m_bLoadError;
        bool m_bLoadErrorM00;
        bool m_bForceBriefingM00;
        bool m_pad6A4F;

        // Exclude anims shit
        // (I Still don't know what the fuck is happening here)
        struct ZExcludedAnimList {
            void* m_pSentinelNode;    // 0x00
            uint32_t m_iSize;         // 0x04
            uint32_t m_pUnusedPadding;// 0x08
        };
        static_assert(sizeof(ZExcludedAnimList) == 12, "Bad size of ZExcludedAnimList");

        ZExcludedAnimList m_aExcluded[4]; // 0x6A50
        
        uint8_t  m_bAnimContainerFlag;    // 0x6A80
        char     m_pad6A81;               // 0x6A81
        char     m_pad6A82;               // 0x6A82
        char     m_pad6A83;               // 0x6A83
        uint32_t m_alwaysZeroed;          // 0x6A84
    }; // Total size is 0x6A88 (27272)

    RE_VERIFY_SIZE(ZHM3GameData, 0x6A88);
    RE_VERIFY_OFFSET(ZHM3GameData, m_apPlayers, 0x0A20);
    RE_VERIFY_OFFSET(ZHM3GameData, m_ParticleTemplatesList, 0x0A18);
    RE_VERIFY_OFFSET(ZHM3GameData, m_Hitman3, 0x0A40);
    RE_VERIFY_OFFSET(ZHM3GameData, m_Gui, 0x0A50);
    RE_VERIFY_OFFSET(ZHM3GameData, m_rActorCommunication, 0x6990);
    RE_VERIFY_OFFSET(ZHM3GameData, m_pClothTracker, 0x699C);
    RE_VERIFY_OFFSET(ZHM3GameData, m_DialogControl, 0x69A8);
    RE_VERIFY_OFFSET(ZHM3GameData, m_pWeaponUpgradeUtils, 0x69C8);
}

#undef DECLARE_UNKNOWN_TYPE_PTR