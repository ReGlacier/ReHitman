#include <Glacier/GameBase/ZSpaceCell.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    const uint8_t ZSpaceCell::SEEABLES_PER_CELL = 1; // TODO: Finish me
    const uint8_t ZSpaceCell::ILLEGAL_INDEX = 0xFFu; // TODO: Finish me


    ZSpaceCell::ZSpaceCell()
    {
        m_iNumSeeables = 0;
    }

    uint8_t ZSpaceCell::GetSeeableID(uint8_t lIndex) const
    {
        ZASSERT(lIndex < m_iNumSeeables);
        return m_Seeables[lIndex];
    }

    uint8_t ZSpaceCell::GetNumSeeables() const
    {
        return m_iNumSeeables;
    }

    uint8_t ZSpaceCell::AddSeeable(uint8_t lSeeable)
    {
        if (m_iNumSeeables < 63)
        {
            for (int i = 0; i < m_iNumSeeables; ++i)
            {
                if (m_Seeables[i] == lSeeable)
                {
                    ZASSERT(false);
                }
            }

            auto id = m_iNumSeeables;
            ++m_iNumSeeables;
            m_Seeables[id] = lSeeable;
            return id;
        }

        return ILLEGAL_INDEX;
    }

    void ZSpaceCell::RemoveSeeable(uint8_t lSeeable)
    {
        ZASSERT(lSeeable < m_iNumSeeables);
        if (lSeeable < --m_iNumSeeables)
        {
            for (uint8_t i = lSeeable; i < m_iNumSeeables; ++i)
            {
                m_Seeables[i] = m_Seeables[i + 1];
                // TODO: Finish this place after ZCheckVisible will be reversed
                // expected:
                //   ZCheckVisible& checkVisible = ZCheckVisible::Instance();
                //   ZSeeable* pSeeable = checkVisible.GetSeeable(m_Seeables[i]);
                //   pSeeable->DecrementLocatorIndex();
            }
        }
    }
}
