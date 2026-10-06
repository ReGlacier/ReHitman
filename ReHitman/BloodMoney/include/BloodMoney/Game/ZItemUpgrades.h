#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/zstring.h>
#include <cstdint>

namespace Hitman
{
    struct SItemUpgrade
    {
        int32_t m_eItemType;
        int32_t m_iBitShift;
        uint64_t m_iExcludedBy;
        uint32_t m_iTier;
        const char* m_pszName;
        SItemUpgrade* m_pInactivate;
        int32_t m_eCategory;
    };
    static_assert(sizeof(SItemUpgrade) == 0x20, "SItemUpgrade layout");

    class IUpgradeInterface
    {
    public:
        IUpgradeInterface() = default;

        virtual int GetState(int) = 0;
        virtual int GetTier(int) = 0;
        virtual Glacier::zstring GetIconName(int) = 0;
        virtual int32_t GetSize() = 0;
        virtual void Setup() = 0;
        virtual void Select(int) = 0;
        virtual bool IsAffordable(int) = 0;
        virtual Glacier::zstring GetSmallIcon(int) = 0;
        virtual Glacier::zstring GetName(int) = 0;
        virtual Glacier::zstring Get3dGroup() = 0;
        virtual void GetPrice(Glacier::zstring&, int) = 0;
        virtual Glacier::zstring GetLocalePath() = 0;
        virtual Glacier::zstring GetButtonText(int);
        virtual void SetDetailedUpgrade(int);
    };

    class ZItemUpgrades : public IUpgradeInterface
    {
    public:
        ZItemUpgrades();

        int GetState(int) override;
        int GetTier(int) override;
        Glacier::zstring GetIconName(int) override;
        int32_t GetSize() override;
        void Setup() override;
        void Select(int) override;
        bool IsAffordable(int) override;
        Glacier::zstring GetSmallIcon(int) override;
        Glacier::zstring GetName(int) override;
        Glacier::zstring Get3dGroup() override;
        void GetPrice(Glacier::zstring&, int) override;
        Glacier::zstring GetLocalePath() override;
        Glacier::zstring GetButtonText(int) override;
        void SetDetailedUpgrade(int) override;
        bool HasDetails() const;

    private:
        int NormalizeIndex(int) const;
        const SItemUpgrade* GetUpgrade(int) const;

        SItemUpgrade m_aItemUpgrades[11];
    };
    RE_VERIFY_SIZE(ZItemUpgrades, 0x168);
}
