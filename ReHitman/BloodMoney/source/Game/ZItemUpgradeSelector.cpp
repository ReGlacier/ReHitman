#include <BloodMoney/Game/ZItemUpgradeSelector.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/Globals.h>
#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/GUI/Control/ZBUTTON.h>
#include <Glacier/GUI/XMLInterface/ZResourceManager.h>
#include <Glacier/GUI/XMLInterface/ZXMLGUISystem.h>
#include <Glacier/GUI/XMLInterface/System/ZMenuElements.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/System/ZSysInterfaceWintel.h>
#include <cstdio>

namespace Hitman
{
    SWeaponUpgrade::SWeaponUpgrade()
        : m_apGraphic{}
        , m_pParent(nullptr)
        , m_pText(nullptr)
    {
    }

    void SWeaponUpgrade::ReleaseResources(Glacier::ZResourceManager* resourceManager)
    {
        if (!resourceManager)
            return;

        for (auto& graphic : m_apGraphic)
        {
            resourceManager->ReleaseGraphic(graphic);
            graphic = nullptr;
        }
        resourceManager->ReleaseWinGroup(m_pParent);
        resourceManager->ReleaseTextGroup(m_pText);
        m_pParent = nullptr;
        m_pText = nullptr;
    }

    void SWeaponUpgrade::Setup(float* position, const Glacier::zstring& iconName,
        const Glacier::zstring& name, Glacier::ZResourceManager* resourceManager,
        Glacier::ZWINGROUP* parent, const Glacier::zstring& tierName, bool unavailable)
    {
        if (!resourceManager || !parent)
            return;

        if (!m_pParent)
        {
            m_pParent = resourceManager->GetWingroup(parent);
            if (m_pParent)
                m_pParent->SetPos(position[0], position[1], 0.0f);
        }

        if (!m_pText)
        {
            const Glacier::ZVector2 textPos(-5.0f, -48.0f);
            m_pText = resourceManager->GetTextGroup(textPos, nullptr, m_pParent, 1,
                Glacier::FT_MENU, true, Glacier::ECENTER);
        }
        if (m_pText)
            m_pText->SetText(name.c_str());

        const Glacier::ZVector2 zero(0.0f, 0.0f);
        resourceManager->ReleaseGraphic(m_apGraphic[0]);
        resourceManager->ReleaseGraphic(m_apGraphic[1]);
        resourceManager->ReleaseGraphic(m_apGraphic[3]);
        resourceManager->ReleaseGraphic(m_apGraphic[4]);
        m_apGraphic[0] = resourceManager->GetGraphic(zero, nullptr, m_pParent, iconName.c_str(), Glacier::ECENTER, -1);
        m_apGraphic[1] = resourceManager->GetGraphic(zero, nullptr, m_pParent, "FrameGray*", Glacier::ECENTER, -1);
        if (!m_apGraphic[2])
            m_apGraphic[2] = resourceManager->GetGraphic(zero, nullptr, m_pParent, "FrameGray*", Glacier::ECENTER, -1);
        m_apGraphic[3] = resourceManager->GetGraphic(Glacier::ZVector2(22.0f, 63.0f), nullptr,
            m_pParent, tierName.c_str(), Glacier::ECENTER, -1);
        if (unavailable)
            m_apGraphic[4] = resourceManager->GetGraphic(zero, nullptr, m_pParent,
                "RedOverlay*", Glacier::ECENTER, -1);
    }

    void SWeaponUpgrade::SetPos(float* position)
    {
        if (m_pParent)
            m_pParent->SetPos(position[0], position[1], 0.0f);
    }

    void SWeaponUpgrade::SetNumber(int number)
    {
        if (m_pText)
        {
            char text[16]{};
            std::snprintf(text, sizeof(text), "%d", number);
            m_pText->SetText(text);
        }
    }

    void SWeaponUpgrade::HideText(bool hide)
    {
        if (m_pText)
            m_pText->HideRecursive(hide);
    }

    Glacier::ZWINGROUP* SWeaponUpgrade::GetTierGraphic()
    {
        return m_apGraphic[3];
    }

    ZUpgradeSelector::ZUpgradeSelector()
        : Glacier::IGUIElement()
        , m_pWeaponUpgrade(nullptr)
        , m_aBackupSuitCases{}
        , m_abGotBackup{}
        , m_Padding2{}
        , m_iHitmanMoneyBackup(0)
        , m_iFocus(0)
        , m_pResourceManager(nullptr)
        , m_pParent(nullptr)
        , m_pBackground(nullptr)
        , m_pCurrentFocus(nullptr)
        , m_pButton(nullptr)
        , m_pLeftButton(nullptr)
        , m_pRightButton(nullptr)
        , m_pWhiteBorder(nullptr)
        , m_pColorSetNonBuyable(nullptr)
        , m_pColorSetTier(nullptr)
        , m_eLastMove(ENONE)
        , m_aWeaponUpgrades{}
        , m_v2CurrentPos{}
        , m_EaseIn{}
        , m_bAnimating(false)
        , m_Padding3{}
    {
        m_EaseIn.m_iPolyDegree = 2;
        m_EaseIn.m_fCurVal = 0.0f;
        m_EaseIn.m_fDstVal = 0.0f;
        m_EaseIn.m_fStartVal = 0.0f;
        m_EaseIn.m_TimeInterval = Glacier::TIMETYPE(0.3f);
    }

    int ZUpgradeSelector::GetFocus() { return m_iFocus; }
    IUpgradeInterface* ZUpgradeSelector::GetUpgradeInterface() { return m_pWeaponUpgrade; }

    int ZUpgradeSelector::GetUpgradeNumber(int index) const
    {
        if (!m_pWeaponUpgrade || m_pWeaponUpgrade->GetSize() <= 0)
            return 0;
        return index + 1;
    }

    void ZUpgradeSelector::Backup()
    {
        auto* gameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        if (!gameData)
            return;
        m_iHitmanMoneyBackup = gameData->m_LevelLinking.GetHitmanMoney();
        for (int i = 0; i < AVAILABLE_BACKUP_ITEMS; ++i)
        {
            m_abGotBackup[i] = gameData->m_LevelLinking.GetAvailableItem(i, m_aBackupSuitCases[i]) != -1;
        }
    }

    void ZUpgradeSelector::Cancel()
    {
        auto* gameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        if (!gameData)
            return;
        for (int i = 0; i < AVAILABLE_BACKUP_ITEMS; ++i)
        {
            if (m_abGotBackup[i])
                gameData->m_LevelLinking.AddAvailableItem(m_aBackupSuitCases[i], true);
        }
        gameData->m_LevelLinking.IncMoney(static_cast<int>(m_iHitmanMoneyBackup) -
            static_cast<int>(gameData->m_LevelLinking.GetHitmanMoney()));
    }

    void ZUpgradeSelector::MoveLeft()
    {
        if (!m_pWeaponUpgrade)
            return;
        ++m_iFocus;
        if (m_iFocus >= m_pWeaponUpgrade->GetSize())
            m_iFocus = 0;
        m_eLastMove = ELEFT;
        SetupGraphics(ELEFT);
        Glacier::g_pGameData->GetAudioOSDInterface().PlaySound(1);
    }

    void ZUpgradeSelector::MoveRight()
    {
        if (!m_pWeaponUpgrade)
            return;
        --m_iFocus;
        if (m_iFocus < 0)
            m_iFocus = m_pWeaponUpgrade->GetSize() - 1;
        m_eLastMove = ERIGHT;
        SetupGraphics(ERIGHT);
        Glacier::g_pGameData->GetAudioOSDInterface().PlaySound(1);
    }

    void ZUpgradeSelector::Select()
    {
        if (!m_pWeaponUpgrade)
            return;
        Glacier::g_pGameData->GetAudioOSDInterface().PlaySound(3);
        m_pWeaponUpgrade->Select(m_iFocus);
        SetupGraphics(ENONE);
    }

    void ZUpgradeSelector::SetupGraphics(EMovement movement)
    {
        if (!m_pWeaponUpgrade || !m_pResourceManager || !m_pParent)
            return;

        int index = m_iFocus - 2;
        if (movement == ELEFT)
            --index;
        else if (movement == ERIGHT)
            ++index;

        const int size = m_pWeaponUpgrade->GetSize();
        if (size <= 0)
            return;
        for (int i = 0; i < NUM_OF_UPGRADES_ON_SCREEN; ++i)
        {
            int current = index;
            if (current >= size)
                current -= size;
            if (current < 0)
                current += size;
            Glacier::zstring tier;
            tier.format("Tier%i*", m_pWeaponUpgrade->GetTier(current) + 1);
            float position[2] = { static_cast<float>((i + 1) * 92), 0.0f };
            const int state = m_pWeaponUpgrade->GetState(current);
            m_aWeaponUpgrades[i].Setup(position, m_pWeaponUpgrade->GetIconName(current),
                m_pWeaponUpgrade->GetName(current), m_pResourceManager, m_pParent, tier, state == 4);
            m_aWeaponUpgrades[i].SetNumber(GetUpgradeNumber(current));
            m_aWeaponUpgrades[i].HideText(state == 4);
            if (m_pColorSetTier && m_aWeaponUpgrades[i].GetTierGraphic())
                ChangeColor(m_aWeaponUpgrades[i].GetTierGraphic(), m_pColorSetTier,
                    Glacier::ZColorSet::NormalColor);
            ++index;
        }
        m_eLastMove = movement;
        m_EaseIn.m_fCurVal = 0.0f;
        m_EaseIn.m_fStartVal = 0.0f;
        m_EaseIn.m_fDstVal = movement == ELEFT ? -92.0f : movement == ERIGHT ? 92.0f : 0.0f;
        m_bAnimating = movement != ENONE;
    }

    Glacier::ZGUIElementLink ZUpgradeSelector::Setup(float* position, Glacier::ZResourceManager* resourceManager, Glacier::ZWINGROUP* group)
    {
        m_pResourceManager = resourceManager;
        m_pParent = group;
        m_iFocus = 0;
        if (!resourceManager || !group)
            return Glacier::ZGUIElementLink();

        m_pBackground = resourceManager->GetWingroup(group);
        m_pCurrentFocus = resourceManager->GetWingroup(group);
        const Glacier::ZVector2 buttonSize(224.0f, 27.0f);
        m_pButton = resourceManager->GetButton(buttonSize, m_pColorSet, m_pBackground,
            m_iIndex, nullptr, nullptr, Glacier::ELEFT, nullptr, 9, Glacier::FT_MENU, true, false);
        const Glacier::ZVector2 leftPosition(32.0f, 27.0f);
        m_pLeftButton = resourceManager->GetButton(leftPosition, m_pColorSet, m_pBackground,
            GenerateSubId(1), nullptr, nullptr, Glacier::ELEFT, nullptr, 9, Glacier::FT_MENU, true, false);
        const Glacier::ZVector2 rightPosition(519.0f, 27.0f);
        m_pRightButton = resourceManager->GetButton(rightPosition, m_pColorSet, m_pBackground,
            GenerateSubId(2), nullptr, nullptr, Glacier::ELEFT, nullptr, 9, Glacier::FT_MENU, true, false);
        m_v2CurrentPos[0] = position[0];
        m_v2CurrentPos[1] = position[1];
        if (m_pWeaponUpgrade)
        {
            m_pWeaponUpgrade->Setup();
            SetupGraphics(ENONE);
        }
        return Glacier::ZGUIElementLink(230.0f, 98.0f, m_pButton);
    }

    void ZUpgradeSelector::ReleaseResources(Glacier::ZResourceManager* resourceManager)
    {
        for (auto& upgrade : m_aWeaponUpgrades)
            upgrade.ReleaseResources(resourceManager);
        if (resourceManager)
        {
            resourceManager->ReleaseButton(m_pButton);
            resourceManager->ReleaseButton(m_pLeftButton);
            resourceManager->ReleaseButton(m_pRightButton);
            resourceManager->ReleaseGraphic(m_pWhiteBorder);
            resourceManager->ReleaseWinGroup(m_pCurrentFocus);
            resourceManager->ReleaseWinGroup(m_pBackground);
        }
        m_pButton = nullptr;
        m_pLeftButton = nullptr;
        m_pRightButton = nullptr;
        m_pWhiteBorder = nullptr;
        m_pCurrentFocus = nullptr;
        m_pBackground = nullptr;
    }

    void ZUpgradeSelector::Click(Glacier::eZWUserEvents event, int value, Glacier::ZXMLGUISystem* system)
    {
        if (m_bAnimating)
            return;
        if (event == Glacier::eZW_CANCEL)
        {
            if (system)
                system->CloseWindow(false);
            return;
        }
        if (Glacier::g_pSysInterface->m_bUseGameController)
        {
            switch (event)
            {
            case Glacier::eZW_LEFT: MoveLeft(); break;
            case Glacier::eZW_RIGHT: MoveRight(); break;
            case Glacier::eZW_SELECT: Select(); break;
            default: break;
            }
            if (event == Glacier::eZW_SELECT2 && m_pWeaponUpgrade && system
                && m_pWeaponUpgrade->GetState(m_iFocus) != 4)
                system->OpenWindow("WeaponDetailsMenu", true, false);
            return;
        }
        if (event == Glacier::eZW_SELECT)
        {
            if (value == 0) Select();
            else if (value == 1) MoveRight();
            else if (value == 2) MoveLeft();
        }
        else if (event == Glacier::eZW_MWHEELUP || event == Glacier::eZW_LEFT)
            MoveRight();
        else if (event == Glacier::eZW_MWHEELDOWN || event == Glacier::eZW_RIGHT)
            MoveLeft();
    }

    bool ZUpgradeSelector::SetFocus(bool focused)
    {
        if (focused && m_pButton)
            m_pButton->GrabFocus();
        return true;
    }

    void ZUpgradeSelector::Update(bool)
    {
        const bool finished = m_EaseIn.MapFunction() >= 1.0f;
        const bool wasAnimating = m_bAnimating;
        if (wasAnimating)
        {
            float x = m_EaseIn.m_fCurVal;
            if (m_eLastMove == ERIGHT)
                x -= 92.0f;
            for (auto& upgrade : m_aWeaponUpgrades)
            {
                x += 92.0f;
                float position[2] = { x, 0.0f };
                upgrade.SetPos(position);
            }
        }
        if (wasAnimating && finished)
        {
            m_bAnimating = false;
            SetupGraphics(ENONE);
        }
    }

    void ZUpgradeSelector::readParams(const char** params, Glacier::ZMenuElements* elements)
    {
        Glacier::IGUIElement::readParams(params, elements);
        const char* nonBuyable = Glacier::GUI::GetAttr(params, "NonBuyableColorSet", false);
        const char* tier = Glacier::GUI::GetAttr(params, "TierColorSet", false);
        if (elements && nonBuyable)
            m_pColorSetNonBuyable = elements->GetColorSet(nonBuyable);
        if (elements && tier)
            m_pColorSetTier = elements->GetColorSet(tier);
    }

    void ZUpgradeSelector::Invalidate() { SetupGraphics(ENONE); }

    ZItemUpgradeSelector::ZItemUpgradeSelector()
        : ZUpgradeSelector()
        , m_ItemUpgrade()
    {
        m_pWeaponUpgrade = &m_ItemUpgrade;
    }
}
