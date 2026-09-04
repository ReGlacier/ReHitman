#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZGameStats.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZGameData::ZGameData()
        : m_pkBoidSystem(nullptr),
          m_ActorsPool(),
          m_pTrackLinkObjectList(nullptr),
          m_ItemsPool(),
          m_bDisableDust(false),
          m_pParticleTemplates(REFTAB::MakeReftab(16, 0)),
          m_pMenuElements(nullptr),
          m_apPlayerIndex{},
          m_pStats(nullptr),
          m_pSoundDef_OSD(nullptr),
          m_pSoundDef_Effects(nullptr),
          m_bGameJustLoaded(false)
    {
        // TODO: Finish this place after ZBoidSystem will be moved to Glacier
    }

    ZGameData::~ZGameData()
    {
        REFTAB::DeleteReftab(m_pParticleTemplates);
        m_pParticleTemplates = nullptr;
        ZUniMemory::Delete(m_pStats);
        m_pStats = nullptr;
        // TODO: Finish this place after ZBoidSystem will be moved to Glacier
    }

    void ZGameData::LoadSave(ISerializerStream& stream, bool)
    {
        m_bGameJustLoaded = !stream.IsLoading();
    }

    void ZGameData::Init() {}
    void ZGameData::Init2() {}
    void ZGameData::RemapRefs(uint32_t*, uint32_t) {}
    void ZGameData::OnLevelChangeBegin() {}
    void ZGameData::OnLevelChangeLoadDone() const {}
    void ZGameData::OnLevelChangeFinish() const {}
    void ZGameData::PreFrameUpdate() {}
    void ZGameData::PostFrameUpdate() {}
    void ZGameData::AddGeomToCheckPoint(ZGEOM*) {}
    void ZGameData::AddEventToCheckPoint(ZEventBase*) {}
    void ZGameData::AddItemOnGround(ZItem*) {}
    void ZGameData::RemoveItemOnGround(ZItem*) {}
    void ZGameData::CheckPointSave(ZCheckPointBuffer&) {}
    void ZGameData::CheckPointLoad(ZCheckPointBuffer&) {}
    bool ZGameData::IsFrontEnd() const
    {
        return true;
    }

    ZGameStats& ZGameData::GetStats()
    {
        if (!m_pStats)
            m_pStats = ZUniMemory::New<ZGameStats>();
        return *m_pStats;
    }

    void ZGameData::InitExcludedAnimNames(uint32_t*)
    {
        ZASSERT(false);
    }
    ZAnimExclude* ZGameData::GetAnimExclude()
    {
        return nullptr;
    }
    void ZGameData::InitWeaponHandles(uint32_t*)
    {
        ZASSERT(false);
    }
    uint32_t ZGameData::GetTotalWeaponPrimsCount() const
    {
        return 0;
    }
    uint32_t* ZGameData::GetTotalWeaponPrims() const
    {
        return nullptr;
    }
    uint32_t ZGameData::GetUsedWeaponPrimsCount() const
    {
        return 0;
    }
    uint32_t* ZGameData::GetUsedWeaponPrims() const
    {
        return nullptr;
    }
    uint32_t ZGameData::GetBigWeaponPrimsCount() const
    {
        return 0;
    }
    uint32_t* ZGameData::GetBigWeaponPrims() const
    {
        return nullptr;
    }
    void ZGameData::OnDecalRemoval(uint32_t) const {}
    void ZGameData::GetSuitcaseWeaponPrims(int&, uint32_t*) const
    {
        ZASSERT(false);
    }
    bool ZGameData::IsInCutsequence()
    {
        return false;
    }
    const char* ZGameData::LoadGameFailureScene()
    {
        return g_pEngineData->GetSceneName();
    }
    void ZGameData::AddIconToText(zstring&, const char*, bool) {}

    void ZGameData::SetPlayer(ZPlayer* pPlayer, int32_t lIndex)
    {
        ZASSERT(pPlayer && static_cast<uint32_t>(lIndex) < 4);
        if (!m_apPlayerIndex[lIndex])
            m_apPlayerIndex[lIndex] = pPlayer;
        // TODO: Finish this place after ZDllSound player registration will be reversed
    }

    void ZGameData::SetAudioOSDInterface(ZSDOwner* pOwner)
    {
        m_pSoundDef_OSD = pOwner;
        if (pOwner)
            pOwner->SetEnsureOneChannel(true);
    }

    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZGameData*, g_pGameData, 0x0082083C, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZGameDataFactoryBase*, g_pGameDataFactory, 0x00820838, nullptr);
}
