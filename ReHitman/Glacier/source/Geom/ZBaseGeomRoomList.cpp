#include <Glacier/Geom/ZBaseGeomRoomList.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    void ZBaseGeomRoomList::Remove(ZROOM* pRoom)
    {
        for (uint8_t i = 0; i < m_cNrRooms; ++i)
        {
            if (m_pRooms[i] == pRoom)
            {
                m_pRooms[i] = m_pRooms[--m_cNrRooms];
                return;
            }
        }
    }

    void ZBaseGeomRoomList::Init()
    {
        m_cNrRooms = 0;
    }

    ZROOM* ZBaseGeomRoomList::GetRoomNr(uint32_t lRoomNr) const
    {
        ZASSERT(lRoomNr < Count());

        if (lRoomNr >= Count())
            return nullptr;

        return m_pRooms[lRoomNr];
    }

    ZROOM** ZBaseGeomRoomList::GetRoomList()
    {
        return m_pRooms;
    }

    bool ZBaseGeomRoomList::Exists(ZROOM* pRoom) const
    {
        for (uint8_t i = 0; i < m_cNrRooms; ++i)
        {
            if (m_pRooms[i] == pRoom)
                return true;
        }

        return false;
    }

    uint8_t ZBaseGeomRoomList::Count() const
    {
        return m_cNrRooms;
    }

    void ZBaseGeomRoomList::Clear()
    {
        m_cNrRooms = 0;
    }

    bool ZBaseGeomRoomList::Add(ZROOM* pRoom)
    {
        if (m_cNrRooms >= 6)
            return false;

        m_pRooms[m_cNrRooms++] = pRoom;
        return true;
    }
}
