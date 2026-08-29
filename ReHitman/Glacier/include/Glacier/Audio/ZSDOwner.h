#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    //fwds
    class ZGEOM;
    class ZSoundObject;


    struct ZSDOwner
    {
        // methods
        ZSDOwner();
        uint32_t GetEnumSize() const;
        void StopCurrentSound();
        void PlaySound(int _enumIndex);
        void PlaySoundNow(int _enumIndex, ZSoundObject* _pUseMe);
        void PlaySoundNow(int _enumIndex, ZGEOM* _locationGeom, ZSoundObject* _pUseMe);
        void AllowSounds();
        int32_t GetSoundFromEnumIndex(uint8_t _enumIndex);
        void SetEnsureOneChannel(bool bEnsureOnChannel);
        bool IsEnsuringOneChannel() const;


        // members
        int m_iSoundDefinitionIndex;
        bool m_bEnsureOneChannel;
        RE_ADD_PADDING(3);
        float m_Time_AllowedToPlay;
        unsigned int m_rLastPlayed;
    };
    RE_VERIFY_SIZE(ZSDOwner, 0x10);
}
