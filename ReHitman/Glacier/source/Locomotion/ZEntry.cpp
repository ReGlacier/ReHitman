#include <Glacier/Locomotion/ZEntry.h>

#include <Glacier/IK/ZLNKOBJ.h>


namespace Glacier::Locomotion
{
    ZEntry::ZEntry()
        : m_SpeedThreshold{0.0f, 10000.0f}
        , m_Direction{1.0f, 0.0f}
        , m_DistanceThreshold{0.0f, 100000.0f}
        , m_CycleCorrectionSpeed(0.05f)
        , m_Flags(0)
        , m_CompanionEntryTo(-1)
        , m_bSeekFullMoveCycles(false)
    {
    }

    ZEntry* ZEntry::Init(ZAnimTemplatesNames& names, float fMinSpeed, float fMaxSpeed,
        float fMinDistance, float fMaxDistance, const char* pszAnimation)
    {
        m_SpeedThreshold[0] = fMinSpeed;
        m_SpeedThreshold[1] = fMaxSpeed;
        m_DistanceThreshold[0] = fMinDistance;
        m_DistanceThreshold[1] = fMaxDistance;
        m_Direction[0] = 0.0f;
        m_Direction[1] = 10000.0f;
        names.FindAnimVariationHandle(m_AnimHandle, pszAnimation);
        m_Flags = 0;
        m_CompanionEntryTo = -1;
        m_bSeekFullMoveCycles = false;
        m_CycleCorrectionSpeed = 0.05f;
        return this;
    }

    ZEntry* ZEntry::SetFlags(int8_t flags)
    {
        m_Flags = flags;
        return this;
    }

    ZEntry* ZEntry::SetCompanionEntryTo(int8_t entry)
    {
        m_CompanionEntryTo = entry;
        return this;
    }

    ZEntry* ZEntry::SeekFullMoveCycles(bool seek)
    {
        m_bSeekFullMoveCycles = seek;
        return this;
    }

    ZEntry* ZEntry::SetCycleCorrectionSpeed(float speed)
    {
        m_CycleCorrectionSpeed = speed;
        return this;
    }

    void ZEntry::SetParameters(float fMinSpeed, float fMaxSpeed, float fDirectionX,
        float fDirectionY, float fMinDistance, float fMaxDistance)
    {
        m_SpeedThreshold[0] = fMinSpeed;
        m_SpeedThreshold[1] = fMaxSpeed;
        m_Direction[0] = fDirectionX;
        m_Direction[1] = fDirectionY;
        m_DistanceThreshold[0] = fMinDistance;
        m_DistanceThreshold[1] = fMaxDistance;
    }
}
