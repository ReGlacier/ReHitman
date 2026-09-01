#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/Materials/ZTypedef.h>
#include <cstdint>


namespace Glacier
{
    class ZActor;
    class ZAnimExclude;
    class ZMenuElements;
    class ZPlayer;
    struct ZSDOwner;

    class ZGameData
    {
    public:
        ZGameData();
        virtual ~ZGameData();
        virtual void LoadSave(ISerializerStream& stream, bool bSaving);
        virtual void Init();
        virtual void Init2();
        virtual void RemapRefs(uint32_t* pRemapTable, uint32_t lCount);
        virtual void OnLevelChangeBegin();
        virtual void OnLevelChangeLoadDone() const;
        virtual void OnLevelChangeFinish() const;
        virtual void PreFrameUpdate();
        virtual void PostFrameUpdate();
        virtual void AddGeomToCheckPoint(ZGEOM* pGeom);
        virtual void AddEventToCheckPoint(ZEventBase* pEvent);
        virtual void AddItemOnGround(ZItem* pItem);
        virtual void RemoveItemOnGround(ZItem* pItem);
        virtual void CheckPointSave(ZCheckPointBuffer& buffer);
        virtual void CheckPointLoad(ZCheckPointBuffer& buffer);
        virtual TEnumID GetAmmoEnumId(const char* psName) = 0;
        virtual bool IsFrontEnd() const;
        virtual ZGameStats& GetStats();
        virtual void InitExcludedAnimNames(uint32_t* pHandles);
        virtual ZAnimExclude* GetAnimExclude();
        virtual void InitWeaponHandles(uint32_t* pHandles);
        virtual uint32_t GetTotalWeaponPrimsCount() const;
        virtual uint32_t* GetTotalWeaponPrims() const;
        virtual uint32_t GetUsedWeaponPrimsCount() const;
        virtual uint32_t* GetUsedWeaponPrims() const;
        virtual uint32_t GetBigWeaponPrimsCount() const;
        virtual uint32_t* GetBigWeaponPrims() const;
        virtual void OnDecalRemoval(uint32_t lPrim) const;
        virtual void GetSuitcaseWeaponPrims(int& lCount, uint32_t* pPrims) const;
        virtual bool IsInCutsequence();
        virtual const char* LoadGameFailureScene();

        void SetTrackLinkObjectList(ZLIST* pList) { m_pTrackLinkObjectList = pList; }
        ZLIST* GetTrackLinkObjectList() const { return m_pTrackLinkObjectList; }
        void SetPlayer(ZPlayer* pPlayer, int32_t lIndex);
        ZPlayer* GetPlayer(int32_t lIndex) const { return m_apPlayerIndex[lIndex]; }
        void SetAudioOSDInterface(ZSDOwner* pOwner);
        ZSDOwner& GetAudioOSDInterface() const { return *m_pSoundDef_OSD; }
        void SetAudioEffectsInterface(ZSDOwner* pOwner) { m_pSoundDef_Effects = pOwner; }
        ZSDOwner& GetAudioEffectsInterface() const { return *m_pSoundDef_Effects; }

        void* m_pkBoidSystem;
        ZStaticVector<ZActor*, 512> m_ActorsPool;
        ZLIST* m_pTrackLinkObjectList;
        ZStaticVector<ZGEOM*, 128> m_ItemsPool;
        bool m_bDisableDust;
        RE_ADD_PADDING(3);
        REFTAB* m_pParticleTemplates;
        ZMenuElements* m_pMenuElements;
        ZPlayer* m_apPlayerIndex[4];
        ZGameStats* m_pStats;
        ZSDOwner* m_pSoundDef_OSD;
        ZSDOwner* m_pSoundDef_Effects;
        bool m_bGameJustLoaded;
        RE_ADD_PADDING(3);

    protected:
        virtual void AddIconToText(zstring& text, const char* psIcon, bool bGamePad);
    };
    RE_VERIFY_SIZE(ZGameData, 0xA40);
    RE_VERIFY_OFFSET(ZGameData, m_pTrackLinkObjectList, 0x80C);
    RE_VERIFY_OFFSET(ZGameData, m_bDisableDust, 0xA14);
    RE_VERIFY_OFFSET(ZGameData, m_pParticleTemplates, 0xA18);

    class ZGameDataFactoryBase
    {
    public:
        virtual void CreateGameData() = 0;
        virtual void DestroyGameData() = 0;
    };
    RE_VERIFY_SIZE(ZGameDataFactoryBase, 0x4);

    STATIC_GLOBAL_CLASS_INSTANCE(ZGameData*, g_pGameData);
    STATIC_GLOBAL_CLASS_INSTANCE(ZGameDataFactoryBase*, g_pGameDataFactory);
}
