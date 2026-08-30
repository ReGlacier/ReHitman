#include <Glacier/Audio/ZSoundObject.h>
#include <Glacier/Audio/ZSoundObjectControllers.h>

#include <cstring>

namespace Glacier
{
    ZSoundObject::ZSoundObject()
        : m_vBFormat{}
        , m_vLocalPos{}
        , m_vLocalOrient{}
        , m_vPosition{}
        , m_vOrientation{0.0f, 0.0f, 1.0f}
        , m_pPoseData(nullptr)
        , m_pGroup(nullptr)
        , m_rGeomRef(0)
        , m_rParent(0)
        , m_rMaster(0)
        , m_lKey(0)
        , m_lActiveSlaves(0)
        , m_lSoundFlags(0)
        , m_lDeltaPitch(0)
        , m_lPitch(0)
        , m_lPan(0)
        , m_rSound(0)
        , m_lOrigSound(0)
        , m_fFadeStartTime(0.0f)
        , m_fFadeInterval(0.0f)
        , m_fFadeDestination(0.0f)
        , m_fCreateTime()
        , m_fTimeLeft(0.0f)
        , m_fLatency(0.0f)
        , m_fDelay(0.0f)
        , m_fStartOffset(0.0f)
        , m_lCurrentSeqIndex(0)
        , m_fPrio(0.0f)
        , m_lChainIdxGroup(0)
        , m_eSourceType(SOURCE_2D)
        , m_eState(0)
        , m_dwPriority(2)
        , m_lNumPathes(0)
    {
    }

    ZSoundObject::~ZSoundObject()
    {
        if (m_pControllers)
            m_pControllers->Clear();
        Finalize();
    }

    void ZSoundObject::Initalize()
    {
        m_fVolumeAttenuation[0] = 1.0f;
        m_fVolumeAttenuation[1] = 1.0f;
        m_fVolumeDistance = 1.0f;
        m_fInnerConeAngle = 360.0f;
        m_fOuterConeAngle = 360.0f;
        m_lSoundFlags = (m_lSoundFlags & ~0x36u) | 0x26;
        m_lChainIdxCanPlayIn = 0;
        m_lChainIdxDontPlayIn = 0;
        m_lChainIdxNotifyTab = 0;
        m_lChainIdxSlaves = 0;
        m_lChainIdxGroup = 0;
        m_pGroup = nullptr;
        m_rGeomRef = 0;
        m_iVolume = 100;
        m_lLoops = 0;
        m_OuterConeVolume = 0;
        m_lDeltaPitch = -m_lPitch;
        m_lPitch = 0;
    }

    void ZSoundObject::Finalize()
    {
    }

    void ZSoundObject::SetAutoDelete(bool _enabled)
    {
        m_lSoundFlags = _enabled ? m_lSoundFlags | 2 : m_lSoundFlags & ~2u;
    }

    void ZSoundObject::SetLooping(bool _enabled)
    {
        m_lSoundFlags = _enabled ? m_lSoundFlags | 8 : m_lSoundFlags & ~8u;
    }

    void ZSoundObject::AddPath(int _path)
    {
        if (m_lNumPathes < 8)
            m_PathIndices[m_lNumPathes++] = _path;
    }

    void ZSoundObject::SetLowpassCross(float _value)
    {
        m_lLowpassPct = static_cast<int8_t>(_value * 127.0f);
    }

    float ZSoundObject::GetLowpassCross() const
    {
        return m_lLowpassPct / 127.0f;
    }

    void ZSoundObject::SetFadeOut(float _fInterval, float _fWait, float _destination)
    {
        // TODO: Finish me
    }
}
