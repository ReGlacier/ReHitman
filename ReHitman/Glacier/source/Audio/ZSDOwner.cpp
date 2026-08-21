#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/Audio/ZSDOwner.h>


namespace Glacier
{
    ZSDOwner::ZSDOwner()
    {
        m_iSoundDefinitionIndex = 0;
        m_bEnsureOneChannel = false;
        m_Time_AllowedToPlay = 0.0f;
        m_rLastPlayed = 0;
        StopCurrentSound();
    }

    uint32_t ZSDOwner::GetEnumSize() const
    {
        if (m_iSoundDefinitionIndex && g_pSysInterface->m_pSoundDll)
        {
            // TODO: Finish me
        }

        return 0;
    }

    void ZSDOwner::StopCurrentSound()
    {
        if (IsEnsuringOneChannel())
        {
            const TIMETYPE tt = m_Time_AllowedToPlay;

            if (g_pSysInterface->GetRealTime() < tt)
            {
                auto* pLastPlayed = g_pEngineData->SRefToPtr(m_rLastPlayed);
                if (pLastPlayed)
                {
                    // TODO: Uncomment after ZSoundObject reversed
                    // ZUniMemory::Delete(pLastPlayed);
                }

                m_rLastPlayed = 0;
            }
        }

        AllowSounds();
    }

    void ZSDOwner::PlaySound(int _enumIndex)
    {
        // TODO: Finish me
    }

    void ZSDOwner::PlaySoundNow(int _enumIndex, ZSoundObject* _pUseMe)
    {
        // TODO: Finish me
    }

    void ZSDOwner::PlaySoundNow(int _enumIndex, ZGEOM* _locationGeom, ZSoundObject* _pUseMe)
    {
        // TODO: Finish me
    }

    void ZSDOwner::AllowSounds()
    {
        m_Time_AllowedToPlay = g_pSysInterface->GetRealTime();
    }

    int32_t ZSDOwner::GetSoundFromEnumIndex(uint8_t _enumIndex)
    {
        // TODO: Finish me
        return 0;
    }

    void ZSDOwner::SetEnsureOneChannel(bool bEnsureOnChannel)
    {
        m_bEnsureOneChannel = bEnsureOnChannel;
    }

    bool ZSDOwner::IsEnsuringOneChannel() const
    {
        return m_bEnsureOneChannel;
    }
}
