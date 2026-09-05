#include <BloodMoney/Game/ZHM3GameData.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/Materials/BS_Runtime.h>


namespace Hitman::BloodMoney
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
