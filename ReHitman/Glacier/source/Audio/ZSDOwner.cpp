#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Audio/ZDllSound.h>
#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/Geom/ZGEOM.h>


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

    ZSoundObject* ZSDOwner::PlaySound(int _enumIndex)
    {
        auto* pSoundDll = g_pSysInterface->GetSoundDll();
        if (!pSoundDll)
            return nullptr;

        const TIMETYPE tt = m_Time_AllowedToPlay;
        if (IsEnsuringOneChannel() && g_pSysInterface->GetRealTime() < tt)
            return nullptr;

        if (!m_iSoundDefinitionIndex)
            return nullptr;

        const int32_t soundIndex = GetSoundFromEnumIndex(static_cast<uint8_t>(_enumIndex));
        if (!soundIndex)
            return nullptr;

        const ZREF soundRef = pSoundDll->AddSound2d(soundIndex);
        auto* pSoundObject = g_pEngineData->SRefToPtr(soundRef);
        if (pSoundObject)
        {
            if (pSoundObject->m_fTimeLeft < 0.3f)
                pSoundObject->m_fTimeLeft = 0.3f;
            m_Time_AllowedToPlay = static_cast<float>(g_pSysInterface->GetRealTime()) + pSoundObject->m_fTimeLeft;
        }
        if (IsEnsuringOneChannel())
            m_rLastPlayed = soundRef;

        return pSoundObject;
    }

    ZSoundObject* ZSDOwner::PlaySoundNow(int _enumIndex, ZSoundObject* _pUseMe)
    {
        auto* pSoundDll = g_pSysInterface->GetSoundDll();
        if (!pSoundDll)
            return nullptr;

        const TIMETYPE tt = m_Time_AllowedToPlay;
        if (IsEnsuringOneChannel() && g_pSysInterface->GetRealTime() < tt)
            return nullptr;

        if (!m_iSoundDefinitionIndex)
            return _pUseMe;

        const int32_t soundIndex = GetSoundFromEnumIndex(static_cast<uint8_t>(_enumIndex));
        if (!soundIndex)
            return _pUseMe;

        ZSoundObject* pSoundObject = _pUseMe;
        if (pSoundObject)
        {
            pSoundObject->m_lOrigSound = soundIndex;

            // TODO: Finish this place after ZSoundObject::Start will be reversed
            // ZSoundObject::Start(pSoundObject);
        }
        else
        {
            const ZREF soundRef = pSoundDll->AddSound2d(soundIndex);
            pSoundObject = g_pEngineData->SRefToPtr(soundRef);
        }

        if (pSoundObject)
        {
            if (pSoundObject->m_fTimeLeft < 0.3f)
                pSoundObject->m_fTimeLeft = 0.3f;
            m_Time_AllowedToPlay = static_cast<float>(g_pSysInterface->GetRealTime()) + pSoundObject->m_fTimeLeft;
        }
        if (IsEnsuringOneChannel())
            m_rLastPlayed = pSoundObject ? g_pEngineData->SPtrToRef(pSoundObject) : 0;

        return pSoundObject;
    }

    ZSoundObject* ZSDOwner::PlaySoundNow(int _enumIndex, ZGEOM* _locationGeom, ZSoundObject* _pUseMe)
    {
        auto* pSoundDll = g_pSysInterface->GetSoundDll();
        if (!pSoundDll)
            return nullptr;

        const TIMETYPE tt = m_Time_AllowedToPlay;
        if (IsEnsuringOneChannel() && g_pSysInterface->GetRealTime() < tt)
            return nullptr;

        if (!m_iSoundDefinitionIndex)
            return _pUseMe;

        const int32_t soundIndex = GetSoundFromEnumIndex(static_cast<uint8_t>(_enumIndex));
        if (!soundIndex)
            return _pUseMe;

        if (_pUseMe)
        {
            _pUseMe->m_vLocalPos = _locationGeom->Pos();
            _pUseMe->m_vLocalOrient = _locationGeom->Mat().data;
            _pUseMe->m_lOrigSound = soundIndex;
            _pUseMe->m_rParent = _locationGeom->GetRef();

            // TODO: Finish this place after ZSoundObject::Start will be reversed
            // ZSoundObject::Start(_pUseMe);
        }
        else
        {
            ZMat3x3 mat = _locationGeom->Mat();
            ZVector3 pos = _locationGeom->Pos();
            const ZREF soundRef = pSoundDll->AddSound3d(_locationGeom, soundIndex, mat.data, pos);
            _pUseMe = g_pEngineData->SRefToPtr(soundRef);
        }

        if (!_pUseMe)
            return nullptr;

        const SWave* pWave = _pUseMe->GetWave();
        const float fDuration = pWave ? pWave->m_fDuration : 0.0f;
        m_Time_AllowedToPlay = static_cast<float>(g_pSysInterface->GetRealTime()) + fDuration;

        return _pUseMe;
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
