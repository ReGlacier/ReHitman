#include <Glacier/Data/ZGameStats.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/System/ZSysInterface.h>
#include <algorithm>


namespace Glacier
{
    ZGameStats::ZGameStats()
        : m_iStats_CurrentShotCount(0)
        , m_pad06(0)
        , m_iStats_LastShotTime(-1.0f)
    {
    }

    ZGameStats::~ZGameStats() = default;

    void ZGameStats::IncreaseCurrentShotCount()
    {
        ++m_iStats_CurrentShotCount;
        m_iStats_LastShotTime = static_cast<float>(g_pSysInterface->FrameTime.secs) * (1.0f / 1024.0f);
    }

    void ZGameStats::DecreaseCurrentShotCount()
    {
        --m_iStats_CurrentShotCount;
    }

    uint8_t ZGameStats::GetIntensity() const
    {
        if (m_iStats_LastShotTime == -1.0f)
            return 0;

        const float elapsed = std::min(
            static_cast<float>(static_cast<int32_t>(g_pSysInterface->FrameTime.secs
                - static_cast<uint64_t>(m_iStats_LastShotTime * 1024.0f))) * (1.0f / 1024.0f),
            55.0f);
        uint8_t intensity = static_cast<uint8_t>((55.0f - elapsed) * (100.0f / 55.0f));

        // TODO: Replace the proven PC ZPlayer control offset after ZPlayer will be reversed.
        const void* player = g_pGameData ? g_pGameData->GetPlayer(0) : nullptr;
        const uint32_t playerControl = player
            ? *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(player) + 0x3D8)
            : 0;
        if (intensity < 50 && (playerControl & 0x400u) == 0)
            intensity = 50;
        return intensity;
    }
}
