#include <Glacier/Locomotion/ZSet.h>

#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier::Locomotion
{
    ZSet::ZSet()
        : m_iEntries(1)
        , m_iTransitions(0)
        , m_fTurnSpeed(12.0f)
        , m_DefaultState(-1)
        , m_AccelerationFactor(1.0f)
        , m_DecelerationFactor(1.0f)
    {
    }

    ZEntry& ZSet::GetEntry(int index)
    {
        ZASSERT(index >= 0 && index < m_iEntries);
        return m_Entries[index];
    }

    ZTransition& ZSet::GetTransition(int index)
    {
        ZASSERT(index >= 0 && index < m_iTransitions);
        return m_Transitions[index];
    }

    int ZSet::NewEntry()
    {
        ZASSERT(m_iEntries < MaxEntries);
        return m_iEntries++;
    }

    int ZSet::NewTransition()
    {
        ZASSERT(m_iTransitions < MaxTransitions);
        return m_iTransitions++;
    }

    void ZSet::SetOpenDoorAnim(ZAnimTemplatesNames& names, const char* pszAnimation)
    {
        names.FindAnimVariationHandle(m_OpenDoorAnim, pszAnimation);
    }

    void ZSet::SetOpenDoubleDoorAnim(ZAnimTemplatesNames& names, const char* pszAnimation)
    {
        names.FindAnimVariationHandle(m_OpenDoubleDoorAnim, pszAnimation);
    }

    void ZSet::SetRunOpenDoorAnim(ZAnimTemplatesNames& names, const char* pszAnimation)
    {
        names.FindAnimVariationHandle(m_RunOpenDoorAnim, pszAnimation);
    }

    void ZSet::SetRunOpenDoubleDoorAnim(ZAnimTemplatesNames& names, const char* pszAnimation)
    {
        names.FindAnimVariationHandle(m_RunOpenDoubleDoorAnim, pszAnimation);
    }
}
