#pragma once

#include <Glacier/Audio/ZAudioType.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZREF.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    namespace SoundObject
    {
        class ZControllers;
    }

    class ZGEOM;
    class ZROOM;

    class ZSoundObject
    {
    public:
        enum ESourceType : uint32_t
        {
            SOURCE_2D = 3
        };

        // methods
        ZSoundObject();
        ~ZSoundObject();
        void Initalize();
        void Finalize();

        void SetVolumeDistance(float _volume) { m_fVolumeDistance = _volume; }
        float GetVolumeDistance() const { return m_fVolumeDistance; }
        void SetVolumeAttenuation(float _volume) { m_fVolumeAttenuation[0] = _volume; }
        float GetVolumeAttenuation() const { return m_fVolumeAttenuation[0]; }
        float GetCalculatedVolumeAttenuation() const { return m_fVolumeAttenuation[1]; }
        void SetPitch(int _pitch) { m_lPitch = _pitch; }
        int GetPitch() const { return m_lPitch; }
        int GetCalculatedPitch() const { return m_lPitch + m_lDeltaPitch; }
        void SetPan(int _pan) { m_lPan = _pan; }
        int GetPan() const { return m_lPan; }
        void SetMinDistance(float _distance) { m_fMinDist = _distance; }
        float GetMinDistance() const { return m_fMinDist; }
        void SetMinDistanceFactor(float _factor) { m_fMinDistFactor = _factor; }
        float GetMinDistanceFactor() const { return m_fMinDistFactor; }
        float GetMaxDistance() const { return m_fMaxDist; }
        void SetGroup(SSoundGroup* _group) { m_pGroup = _group; }
        SSoundGroup* GetGroup() { return m_pGroup; }
        const SSoundGroup* GetGroup() const { return m_pGroup; }
        bool IsPlaying() const { return (m_lSoundFlags & 0x4000) != 0; }
        void SetDelay(float _delay) { m_fDelay = _delay; }
        float GetDelay() const { return m_fDelay; }
        void SetStartOffset(float _offset) { m_fStartOffset = _offset; }
        float GetStartOffset() const { return m_fStartOffset; }
        void SetTimeLeft(float _time) { m_fTimeLeft = _time; }
        float GetTimeLeft() const { return m_fTimeLeft; }
        void SetLatency(float _latency) { m_fLatency = _latency; }
        float GetLatency() const { return m_fLatency; }
        void SetDeltaPitch(int _pitch) { m_lDeltaPitch = _pitch; }
        int GetDeltaPitch() const { return m_lDeltaPitch; }
        ESourceType GetSourceType() const { return m_eSourceType; }
        void SetSourceType(ESourceType _type) { m_eSourceType = _type; }
        bool IsSource2D() const { return m_eSourceType == SOURCE_2D; }
        uint32_t GetPriority() const { return m_dwPriority; }
        void SetPriority(uint32_t _priority) { m_dwPriority = static_cast<uint8_t>(_priority); }
        float GetCalculatedPriority() const { return m_fPrio; }
        void SetCalculatedPriority(float _priority) { m_fPrio = _priority; }
        void SetAutoDelete(bool _enabled);
        bool HasAutoDelete() const { return (m_lSoundFlags & 2) != 0; }
        void SetSoundIdx(int _sound) { m_rSound = _sound; }
        int GetSoundIdx() const { return m_rSound; }
        int GetOrigSoundIdx() const { return m_lOrigSound; }
        void SetLoops(uint8_t _loops) { m_lLoops = _loops; }
        uint8_t GetLoops() const { return m_lLoops; }
        void SetLooping(bool _enabled);
        bool IsLooping() const { return (m_lSoundFlags & 8) != 0; }
        void SetCreateTime(TIMETYPE _time) { m_fCreateTime = _time; }
        TIMETYPE GetCreateTime() const { return m_fCreateTime; }
        void SetPoseData(char* _data) { m_pPoseData = _data; }
        char* GetPoseData() const { return m_pPoseData; }
        ZVector3& GetLocalPosition() { return m_vLocalPos; }
        ZVector3& GetWorldPosition() { return m_vPosition; }
        void AddPath(int _path);
        void ResetPathInfo() { m_lNumPathes = 0; }
        int GetPath(int _index) const { return m_PathIndices[_index]; }
        int NumPathes() const { return m_lNumPathes; }
        void SetLowpassCross(float _value);
        float GetLowpassCross() const;
        int GetLowpassValue() const { return m_lLowpassPct; }
        void SetBufferId(char _id) { m_lBufferId = _id; }
        int8_t GetBufferId() const { return m_lBufferId; }
        void ClearBufferId() { m_lBufferId = -1; }
        bool BufferIdValid() const { return m_lBufferId >= 0; }
        void SetFadeOut(float _fInterval, float _fWait, float _destination);

        // members
        RE_ADD_PADDING(0x24);
        SoundObject::ZControllers* m_pControllers;
        ZVector4 m_vBFormat;
        ZVector3 m_vLocalPos;
        ZVector3 m_vLocalOrient;
        ZVector3 m_vPosition;
        ZVector3 m_vOrientation;
        char* m_pPoseData;
        SSoundGroup* m_pGroup;
        ZREF m_rGeomRef;
        ZREF m_rParent;
        ZREF m_rMaster;
        ZREF m_lKey;
        uint32_t m_lActiveSlaves;
        uint32_t m_lSoundFlags;
        uint32_t m_lChainIdxCanPlayIn;
        uint32_t m_lChainIdxDontPlayIn;
        uint32_t m_lChainIdxNotifyTab;
        uint32_t m_lChainIdxNotifyStarted;
        uint32_t m_lChainIdxSlaves;
        int32_t m_lDeltaPitch;
        int32_t m_lPitch;
        int32_t m_lPan;
        int32_t m_rSound;
        int32_t m_lOrigSound;
        float m_fInnerConeAngle;
        float m_fOuterConeAngle;
        float m_fLPCutOffFreq;
        float m_fMinDist;
        float m_fMinDistFactor;
        float m_fMaxDist;
        float m_fFadeStartTime;
        float m_fFadeInterval;
        float m_fFadeDestination;
        TIMETYPE m_fCreateTime;
        float m_fTimeLeft;
        float m_fLatency;
        float m_fDelay;
        float m_fStartOffset;
        float m_fVolumeAttenuation[2];
        float m_fVolumeDistance;
        uint8_t m_lCurrentSeqIndex;
        RE_ADD_PADDING(3);
        int32_t m_PathIndices[8];
        float m_fPrio;
        uint32_t m_lChainIdxGroup;
        ESourceType m_eSourceType;
        uint32_t m_eState;
        uint8_t m_dwPriority;
        uint8_t m_iMaxDistModel;
        int8_t m_OuterConeVolume;
        uint8_t m_lLoops;
        int8_t m_iVolume;
        int8_t m_lNumPathes;
        int8_t m_lBufferId;
        int8_t m_lLowpassPct;
    };

    RE_VERIFY_OFFSET(ZSoundObject, m_pControllers, 0x24);
    RE_VERIFY_OFFSET(ZSoundObject, m_vBFormat, 0x28);
    RE_VERIFY_OFFSET(ZSoundObject, m_lKey, 0x7C);
    RE_VERIFY_OFFSET(ZSoundObject, m_lSoundFlags, 0x84);
    RE_VERIFY_OFFSET(ZSoundObject, m_fFadeDestination, 0xD0);
    RE_VERIFY_OFFSET(ZSoundObject, m_PathIndices, 0xF8);
    RE_VERIFY_OFFSET(ZSoundObject, m_dwPriority, 0x128);
    RE_VERIFY_SIZE(ZSoundObject, 0x130);
}
