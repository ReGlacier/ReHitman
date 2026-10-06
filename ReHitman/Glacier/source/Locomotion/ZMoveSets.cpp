#include <Glacier/Locomotion/ZMoveSets.h>

#include <Glacier/ZUniAssert.h>


namespace Glacier::Locomotion
{
    ZSet* ZMoveSets::m_pSets[MaxSets]{};
    int ZMoveSets::m_iSize = 0;

    ZSet* ZMoveSets::Get(int index)
    {
        ZASSERT(index >= 0 && index < m_iSize);
        return m_pSets[index];
    }

    void ZMoveSets::Set(int index, ZSet* pSet)
    {
        ZASSERT(index >= 0 && index < MaxSets);
        m_pSets[index] = pSet;
        if (index >= m_iSize)
            m_iSize = index + 1;
    }
}
