#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/GameBase/ZVisibilitySpace.h>
#include <Glacier/GameBase/ZCellCoordinate.h>
#include <Glacier/GameBase/ZSeeable.h>
#include <Glacier/GameBase/SSeer.h>
#include <Glacier/GameBase/ZCellBitmap.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/CBaseEvent.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZItem;
    class ZLNKWHANDS;
    struct COLI;
    class ZDrawDebugRender;
    
    // Seeable type bitmask constants (namespace Vision in checkvisible.cpp).
    namespace Vision
    {
        static constexpr uint32_t MAXSEERS = 0xC8;          // PS2 0x9B77E0
        static constexpr uint32_t MAXSEEABLES = 0xC8;       // PS2 0x9B77E4
        static constexpr float MOVINGUPDATEINTERVAL = 0.5f; // PS2 0x9B77E8 (verify)

        static constexpr uint16_t NONE = 0x0;
        static constexpr uint16_t ITEM = 0x1;
        static constexpr uint16_t ENEMY = 0x2;
        static constexpr uint16_t BODY = 0x4;
        static constexpr uint16_t PLAYER = 0x8;
        static constexpr uint16_t GROUNDDECAL = 0x10;
        static constexpr uint16_t WALLDECAL = 0x20;
        static constexpr uint16_t DECAL = 0x30;             // GROUNDDECAL | WALLDECAL
        static constexpr uint16_t CIVILIAN = 0x40;
        static constexpr uint16_t FRIENDLY = 0x80;
        static constexpr uint16_t ENEMY2 = 0x100;
        static constexpr uint16_t ALL = 0xFFFF;
    }

    class ZCheckVisible : public CBaseEvent<ZLIST>
    {
    public:
        // RTTI
        DECLARE_ROUT_CLASS(ZCheckVisible, ZLIST, CheckVisible, 48, 0);

        // types
        using SeeableID = uint8_t;

        // static
        static ZCheckVisible* m_pCheckVisible;
        static uint16_t m_CallbackWantedMask;

        // vtbl
        ~ZCheckVisible() override;
        void PostSave(ISerializerStream&) override;
        bool PostLoad(ISerializerStream&) override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        void Init() override;
        void Init2() override;
        void CheckPointLoad(ZCheckPointBuffer&) override;
        void FrameUpdate() override;
        void DrawDebugObjects(ZDrawDebugRender*);

        // methods
        ZCheckVisible();

        void Release(bool bFree);

        static ZCheckVisible& Instance();
        static void NukeIt();

        // seeables
        SeeableID AddSeeable(ZGEOM* pObject, float* pPos, uint16_t type);
        SeeableID FindSeeableID(const ZGEOM* pObject) const;
        SeeableID GetFreeSeeableSlot();
        ZSeeable& GetSeeable(uint8_t seeableID);
        void UpdateSeeable(uint8_t seeableID, float* pPos);
        void RemoveSeeable(uint8_t seeableID);
        void RemoveSeeable(const ZGEOM* pObject);

        void AddSeeableItem(ZItem* pItem);
        void UpdateSeeableItem(ZItem* pItem);
        void RemoveSeeableItem(ZItem* pItem);
        void AddSeeableActor(ZLNKWHANDS* pActor, uint16_t type);
        void UpdateSeeableActor(ZLNKWHANDS* pActor);
        void RemoveSeeableActor(ZLNKWHANDS* pActor);
        void AddSeeablePlayer(ZLNKWHANDS* pPlayer);
        void UpdateSeeablePlayer(ZLNKWHANDS* pPlayer);

        // seers
        uint32_t AddSeer(const ZGEOM* pSeer, uint32_t lVisibleTypes);
        void RemoveSeer(const ZGEOM* pSeer);
        uint32_t GetSeerIndex(ZGEOM* pSeer) const;
        bool GetSeer(int* pSeerIndex, const ZGEOM* pSeer) const;
        void GetSeerPosDir(float* pPos, float* pDir, ZGEOM* pSeer, const float* pDefaultPos, bool bUseDefaultPos);
        void SetVisionRange(ZGEOM* pSeer, float fRange);
        void SetFOV(ZGEOM* pSeer, float fFov);
        void SetVerticalFOV(ZGEOM* pSeer, float fFov);
        void SetSeerBlind(ZGEOM* pSeer, bool bBlind);
        void SetVisionOffset(ZGEOM* pSeer, float* pOffset);
        void SetSixthSenseRange(ZGEOM* pSeer, float fRange);
        void SetLightMultiplier(ZGEOM* pSeer, float fMultiplier);
        void SetVisionDistanceMultiplier(float fMultiplier);
        void SetCallbackWantedMask(uint16_t mask);
        void ResetTimers();
        uint32_t SetVisibleTypes(ZGEOM* pSeer, uint32_t lVisibleTypes);
        uint32_t GetVisibleTypes(ZGEOM* pSeer);

        // visibility
        bool IsObjectVisibleToSeer(const ZGEOM* pSeer, ZGEOM* pObject);
        uint32_t GetVisibleSeeables(ZLNKWHANDS* pActor, ZGEOM** pSeeables, uint32_t maxCount, uint16_t types);
        uint32_t GetCurrentSpectators(ZGEOM* pObject, ZGEOM** pSpectators, uint32_t maxCount);
        void NotifySeersOfCellChange(uint8_t seeableID, const ZCellCoordinate& coord);
        void SendNotVisibleToSpectators(uint8_t seeableID);
        void UpdateVisible(uint32_t lSeerNr, uint8_t seeableID, bool bIsVisible, bool bForce);
        void NotifyObjectsNotInCheckedCells(SSeer& seer, ZCellBitmap& visibleCells);
        ZCellCoordinate GetDistanceInCells(float fDist) const;

        // cone helpers
        bool IsCellInVisionCone(ZCellCoordinate& coord, float* pPos, float* pDir, float fCosCone, float fCosVerticalCone);
        void ExcludeCellsByDistance(const ZCellCoordinate& lowCell, const ZCellCoordinate& highCell, ZCellCoordinate& newLowCell, ZCellCoordinate& newHighCell);
        void ExcludeCellsByAngleToAxis(SSeer& seer, float* pPos, float* pAxis, float* pRight, const ZCellCoordinate& cCone, ZCellCoordinate& lowCell, ZCellCoordinate& highCell);

        // decals
        void AddSeeableDecalIfNecessary(const COLI* pColi, float fLifeTime, uint8_t bDecalType, float fDist);
        bool DecalWithinDistance(const COLI* pColi, uint8_t bDecalType, float fDist) const;
        bool CellHasDecalWithinDistance(ZCellCoordinate& coord, uint8_t bDecalType, const float* pPos, float fDist);
        void RemoveAllDecals();
        void PruneSeeableDecals();

        // debug
        void DumpSeers();
        void ShowCones(ZDrawDebugRender* pRender);
        void ShowVision(ZDrawDebugRender* pRender);
        void ShowConeForSeer(ZDrawDebugRender* pRender, SSeer& seer, uint32_t lSeerNr);

        // members
        float m_fVisionDistMultiplier;
        uint32_t m_iCurrentSeer;
        ZCellCoordinate m_cCurrentCell;
        uint8_t m_iCurrentIndex;
        uint8_t m_bResumeSeer;
        uint32_t m_iNumSeers;
        uint32_t m_iNumSeeables;
        uint16_t m_iHighestSeeableDecalID;
        float m_fDeltaTimeFraction;
        ZVisibilitySpace m_Space;
        SSeer m_Seers[Vision::MAXSEERS];
        ZSeeable m_Seeables[Vision::MAXSEEABLES];
        float m_fItemMaxDistance;
        float m_fWallDecalMaxDistance;
        float m_fGroundDecalMaxDistance;
    };
    RE_VERIFY_SIZE(ZCheckVisible, 0x13A8C); // PC verified
    RE_VERIFY_OFFSET(ZCheckVisible, m_fVisionDistMultiplier, 0x30); // PS2 verified
    RE_VERIFY_OFFSET(ZCheckVisible, m_Space, 0x50); // PS2/PC verified
    RE_VERIFY_OFFSET(ZCheckVisible, m_Seers, 0x4080); // PS2/PC verified
    RE_VERIFY_OFFSET(ZCheckVisible, m_Seeables, 0x127C0); // PS2/PC verified
    RE_VERIFY_OFFSET(ZCheckVisible, m_fItemMaxDistance, 0x13A80); // PS2/PC verified
}
