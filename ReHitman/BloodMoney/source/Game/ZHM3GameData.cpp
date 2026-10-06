#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZItemUpgradeSelector.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Com/CGlobalCom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstring>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/Materials/BS_Runtime.h>


namespace Hitman
{
    ZHM3GameData::ZHM3GameData()
        : m_Hitman3(nullptr)
        , m_rPlayer(0)
        , m_LevelControl(nullptr)
        , m_OSD(nullptr)
        , m_Gui(nullptr)
        , m_IngameMap(nullptr)
        , m_Elevators(16, 0)
        , m_FriskGuards(16, 1)
        , m_ItemTemplates(16, 0)
        , m_WantedPosts(16, 0)
    {
        Glacier::g_pGameData = this;
    }

    Glacier::TEnumID ZHM3GameData::GetAmmoEnumId(const char* psName)
    {
        return Glacier::BS_Runtime::ZMaterialDescriptionDB::Instance().GetEnumId(
            "scene:AllLevels/Weapons#/Engine/Objects/Weapons/Ammo",
            psName);
    }

    ZHM3LevelControl* ZHM3GameData::GetLevelControl() const
    {
        return m_LevelControl;
    }

    // PC 0x69D9E0. Walks m_Elevators and returns the one closest (in the XZ plane) to rID.
    Glacier::ZREF ZHM3GameData::FindClosestElevator(Glacier::ZREF rID, float fMaxDist)
    {
        if (fMaxDist < 1000.0f)
            fMaxDist = 1000.0f;

        if (!m_Elevators.Count())
            return 0;

        Glacier::ZGEOM* pGeom = Glacier::ZGEOM::RefToPtr(rID);
        if (!pGeom)
            return 0;

        Glacier::ZMat3x3 mat;
        Glacier::ZVector3 pos;
        pGeom->GetRootTM(mat, pos);

        float fBestDistSq = fMaxDist * fMaxDist;
        Glacier::ZREF rResult = 0;

        for (auto rElevator : m_Elevators.As<Glacier::ZREF>())
        {
            Glacier::ZGEOM* pElevator = Glacier::ZGEOM::RefToPtr(rElevator);
            if (!pElevator)
                continue;

            Glacier::ZMat3x3 matE;
            Glacier::ZVector3 posE;
            pElevator->GetRootTM(matE, posE);

            // The Y axis is ignored so a body matches the elevator directly below/above it.
            posE.y = pos.y;

            const Glacier::ZVector3 diff = pos - posE;
            const float fDistSq = Glacier::vdot(diff.Get(), diff.Get());
            if (fDistSq < fBestDistSq)
            {
                fBestDistSq = fDistSq;
                rResult = rElevator;
            }
        }

        return rResult;
    }


    // PC profile offsets are relative to m_Profile (ZLevelLinking + 8).
    namespace
    {
        constexpr int kProfileStride = 0x16F0;
        constexpr int kDifficultyOffset = 0x5C04;

        int ProfileDifficulty(const ZLevelLinking& linking)
        {
            int difficulty;
            std::memcpy(&difficulty, linking.m_Profile + kDifficultyOffset, sizeof(difficulty));
            return difficulty;
        }

        int ProfileInt(const ZLevelLinking& linking, int offset)
        {
            int value;
            std::memcpy(&value, linking.m_Profile + offset + kProfileStride * ProfileDifficulty(linking), sizeof(value));
            return value;
        }

        void SetProfileInt(ZLevelLinking& linking, int offset, int value)
        {
            std::memcpy(linking.m_Profile + offset + kProfileStride * ProfileDifficulty(linking), &value, sizeof(value));
        }
    }

    uint32_t ZLevelLinking::GetHitmanMoney() const
    {
        return static_cast<uint32_t>(ProfileInt(*this, 0x74));
    }

    uint32_t ZLevelLinking::GetHitmanTotalMoney() const
    {
        uint32_t total = 0;
        for (int offset = 0; offset < 0xD9C; offset += 0x10C)
            total += static_cast<uint32_t>(ProfileInt(*this, 0x120 + offset));
        return total;
    }

    int ZLevelLinking::GetAvailableItem(int itemType, sSuitcaseItem& item) const
    {
        const int count = ProfileInt(*this, 0x70);
        const auto* entries = m_Profile + kProfileStride * ProfileDifficulty(*this) + 0xE20;
        for (int i = 0; i < count; ++i)
        {
            sSuitcaseItem candidate;
            std::memcpy(&candidate, entries + i * 0x20, sizeof(candidate));
            if (candidate.m_eItemType == itemType)
            {
                item = candidate;
                return i;
            }
        }
        return -1;
    }

    bool ZLevelLinking::AddAvailableItem(sSuitcaseItem item, bool updateAvailable)
    {
        const int count = ProfileInt(*this, 0x70);
        if (count == 64)
            return false;
        auto* entries = m_Profile + kProfileStride * ProfileDifficulty(*this) + 0xE20;
        sSuitcaseItem previous;
        const int index = GetAvailableItem(item.m_eItemType, previous);
        if (index == -1)
        {
            std::memcpy(entries + count * 0x20, &item, sizeof(item));
            SetProfileInt(*this, 0x70, count + 1);
            if (item.m_bInSuitcase)
                SetProfileInt(*this, 0x6C, ProfileInt(*this, 0x6C) + 1);
        }
        else
        {
            previous.m_lNumAmmo = item.m_lNumAmmo;
            if (item.m_lUpgradeMask != 0xFFFFFFFFULL)
                previous.m_lUpgradeMask = item.m_lUpgradeMask;
            if (updateAvailable)
                previous.m_lUpgradesAvailable = item.m_lUpgradesAvailable;
            if (previous.m_bInSuitcase && !item.m_bInSuitcase)
                SetProfileInt(*this, 0x6C, ProfileInt(*this, 0x6C) - 1);
            else if (!previous.m_bInSuitcase && item.m_bInSuitcase)
                SetProfileInt(*this, 0x6C, ProfileInt(*this, 0x6C) + 1);
            // PC preserves the old in-suitcase byte when replacing an entry.
            std::memcpy(entries + index * 0x20, &previous, sizeof(previous));
        }
        auto* com = Glacier::ZEngineDataBase::GetGlobalCom();
        m_pCom = com;
        com->SetVal("dataProfile", reinterpret_cast<const char*>(m_Profile), 24208);
        return true;
    }

    void ZLevelLinking::IncMoney(int amount)
    {
        const int money = ProfileInt(*this, 0x74) + amount;
        SetProfileInt(*this, 0x74, money < 0 ? 0 : money);
    }

    uint32_t ZMoneySystem::UpgradePrice(int tier) const
    {
        ZASSERT(static_cast<uint32_t>(tier) <= 4);
        return static_cast<uint32_t>(m_sPrices.iTierUpgradePrice[tier]);
    }

    int ZMoneySystem::GetCurrentTier() const
    {
        const auto* gameData = static_cast<const ZHM3GameData*>(Glacier::g_pGameData);
        const uint32_t total = gameData->m_LevelLinking.GetHitmanTotalMoney();
        int tier = 0;
        while (tier < 4 && total >= static_cast<uint32_t>(m_sPrices.iTierPrices[tier + 1]))
            ++tier;
        return tier;
    }

    class ZGameDataFactory final : public Glacier::ZGameDataFactoryBase
    {
    public:
        void CreateGameData() override
        {
            ZASSERT(!Glacier::g_pGameData);
            Glacier::g_pGameData = ZUniMemory::New<ZHM3GameData>();
        }

        void DestroyGameData() override
        {
            ZUniMemory::Delete(Glacier::g_pGameData);
            Glacier::g_pGameData = nullptr;
        }
    };

    static ZGameDataFactory g_GameDataFactory;

    struct ZGameDataFactoryRegistration
    {
        ZGameDataFactoryRegistration() { Glacier::g_pGameDataFactory = &g_GameDataFactory; }
    };

    static ZGameDataFactoryRegistration g_GameDataFactoryRegistration;
}
