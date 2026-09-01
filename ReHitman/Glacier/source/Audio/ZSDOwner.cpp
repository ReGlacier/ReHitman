#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Audio/ZDllSound.h>
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
                    ZUniMemory::Delete(pLastPlayed);
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
        auto* pSoundDll = g_pSysInterface->GetSoundDll();
        if (!m_iSoundDefinitionIndex || !pSoundDll)
            return 0;

        auto* pPacked = pSoundDll->GetPackedObject(m_iSoundDefinitionIndex);
        if (!pPacked || pPacked->m_Type != ZAudioTypes::Def)
            return 0;

        const auto& definition = pPacked->GetAs<ZAudioType<ZAudioTypes::Def>::ZPacked>();
        if (_enumIndex >= definition.m_Entries.m_lEntryCount)
            return 0;

        auto* pSoundDllImpl = static_cast<ZDllSound*>(pSoundDll);
        const auto* pEntries = reinterpret_cast<const uint32_t*>(
            pSoundDllImpl->m_pPackedData + definition.m_Entries.m_lEntryOffset);
        return static_cast<int32_t>(pEntries[_enumIndex]);
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
