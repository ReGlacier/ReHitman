#include <Glacier/GameBase/ZVisibilitySpace.h>
#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>


namespace Glacier
{
    extern ZDebugInt g_iDumpSeeables;
    extern ZDebugInt g_iShowVisionCells;
    extern ZDebugInt g_iShowCellsForSeer;
    extern ZCellBitmap g_DebugShowCells;

    namespace
    {
        bool ContainsIgnoreCase(const char* text, const char* pattern)
        {
            const size_t patternLength = std::strlen(pattern);
            for (; *text; ++text)
            {
                size_t i = 0;
                while (i < patternLength
                    && text[i]
                    && std::tolower(static_cast<unsigned char>(text[i])) == std::tolower(static_cast<unsigned char>(pattern[i])))
                {
                    ++i;
                }
                if (i == patternLength)
                {
                    return true;
                }
            }
            return false;
        }
    }

    ZVisibilitySpace::ZVisibilitySpace()
    {
        for (int x = 0; x < ZCellCoordinate::XSEGMENTS; ++x)
        {
            for (int y = 0; y < ZCellCoordinate::YSEGMENTS; ++y)
            {
                for (int z = 0; z < ZCellCoordinate::ZSEGMENTS; ++z)
                {
                    new (&m_Sections[x][y][z]) ZSpaceCell();
                }
            }
        }
    }

    void ZVisibilitySpace::Initialize()
    {
        m_vLow = ZVector3(9.9999997e37f, 9.9999997e37f, 9.9999997e37f);
        m_vHigh = ZVector3(-9.9999997e37f, -9.9999997e37f, -9.9999997e37f);

        for (ZBaseGeom* pBase = ZROOT->BaseGeom(); pBase; ZROOT->RecurGetNext(&pBase))
        {
            ZGEOM* pGeom = pBase->GetGeom();
            if (!pGeom || !pGeom->IsDerivedFrom<ZROOM>())
            {
                continue;
            }

            ZVector3 center;
            ZVector3 halfSize;
            if (ContainsIgnoreCase(pGeom->Name(), "VisionSpaceBox"))
            {
                pBase->GetRootBox(center, halfSize);
                m_vHigh = center + halfSize;
                m_vLow = center - halfSize;
                break;
            }

            if (!ContainsIgnoreCase(pGeom->Name(), "backdrop")
                && pGeom != ZROOT
                && !static_cast<ZROOM*>(pGeom)->NotInRoomTree())
            {
                pBase->GetRootBox(center, halfSize);
                m_vHigh.x = std::max(m_vHigh.x, center.x + halfSize.x);
                m_vHigh.y = std::max(m_vHigh.y, center.y + halfSize.y);
                m_vHigh.z = std::max(m_vHigh.z, center.z + halfSize.z);
                m_vLow.x = std::min(m_vLow.x, center.x - halfSize.x);
                m_vLow.y = std::min(m_vLow.y, center.y - halfSize.y);
                m_vLow.z = std::min(m_vLow.z, center.z - halfSize.z);
            }
        }

        m_vCellSize.x = (m_vHigh.x - m_vLow.x) * 0.0625f;
        m_vCellSize.y = m_vHigh.y - m_vLow.y;
        m_vCellSize.z = (m_vHigh.z - m_vLow.z) * 0.0625f;

        m_vCellSizeInv.x = 1.0f / m_vCellSize.x;
        m_vCellSizeInv.y = 1.0f / m_vCellSize.y;
        m_vCellSizeInv.z = 1.0f / m_vCellSize.z;
    }

    ZSpaceCell& ZVisibilitySpace::GetSection(uint32_t x, uint32_t y, uint32_t z)
    {
        return m_Sections[x][y][z];
    }

    ZVector3& ZVisibilitySpace::GetCellSize()
    {
        return m_vCellSize;
    }

    Locator ZVisibilitySpace::UpdateSeeable(uint8_t id, Locator locator, float* pos)
    {
        const ZCellCoordinate coord = FindCell(pos);
        if (coord.IsValid())
        {
            bool bNotifySeers = false;
            if (locator.IsValid())
            {
                const int dx = std::abs(locator.m_X - coord.m_X);
                const int dy = std::abs(locator.m_Y - coord.m_Y);
                const int dz = std::abs(locator.m_Z - coord.m_Z);
                if (dx == 0 && dy == 0 && dz == 0)
                {
                    return locator;
                }
                if (dx >= 2 || dy >= 2 || dz >= 2)
                {
                    bNotifySeers = true;
                }
                GetCell(locator).RemoveSeeable(locator.GetCellIndex());
            }
            else
            {
                bNotifySeers = true;
            }
            ZSpaceCell& cell = GetCell(coord);
            const uint8_t index = cell.AddSeeable(id);
            if (bNotifySeers && index != ZSpaceCell::ILLEGAL_INDEX)
            {
                ZCheckVisible::Instance().NotifySeersOfCellChange(id, coord);
            }
            return Locator(coord, index);
        }
        else
        {
            if (locator.IsValid())
            {
                GetCell(locator).RemoveSeeable(locator.GetCellIndex());
            }
            return Locator::Invalid;
        }
    }

    void ZVisibilitySpace::RemoveSeeable(Locator locator)
    {
        if (locator.IsValid())
        {
            GetCell(locator).RemoveSeeable(locator.GetCellIndex());
        }
    }

    Locator ZVisibilitySpace::AddSeeable(const float* pos, uint8_t id)
    {
        const ZCellCoordinate coord = FindCell(pos);
        ZSpaceCell& cell = GetCell(coord);
        const uint8_t index = cell.AddSeeable(id);
        return Locator(coord, index);
    }

    ZSpaceCell& ZVisibilitySpace::GetCell(const ZCellCoordinate& coord)
    {
        ZASSERT(coord.IsValid());
        return m_Sections[coord.m_X][coord.m_Y][coord.m_Z];
    }

    void ZVisibilitySpace::DumpSeeables()
    {
        g_iDumpSeeables.m_iValue = 0;

        printf("*** Dumping all seeables in all cells in the %dx%dx%d space ***\n", 16, 1, 16);

        ZCellCoordinate coord;
        for (coord.m_X = 0; coord.m_X < ZCellCoordinate::XSEGMENTS; ++coord.m_X)
        {
            for (coord.m_Z = 0; coord.m_Z < ZCellCoordinate::ZSEGMENTS; ++coord.m_Z)
            {
                for (coord.m_Y = 0; coord.m_Y < ZCellCoordinate::YSEGMENTS; ++coord.m_Y)
                {
                    ZSpaceCell& cell = GetCell(coord);
                    if (cell.GetNumSeeables() != 0)
                    {
                        printf("\t* Cell (%d, %d, %d) has %d seeables\n",
                            coord.m_X, coord.m_Y, coord.m_Z, cell.GetNumSeeables());
                        for (uint8_t i = 0; i < cell.GetNumSeeables(); ++i)
                        {
                            ZSeeable& seeable = ZCheckVisible::Instance().GetSeeable(cell.GetSeeableID(i));
                            const char* type = "Unknown\t";
                            switch (seeable.type)
                            {
                                case Vision::ITEM: type = "Item\t"; break;
                                case Vision::ENEMY: type = "Enemy\t"; break;
                                case Vision::BODY: type = "Body\t"; break;
                                case Vision::GROUNDDECAL: type = "Ground Decal"; break;
                                case Vision::WALLDECAL: type = "Wall Decal"; break;
                            }
                            const MYSTR name = seeable.pSeeable && !seeable.IsDecal()
                                ? seeable.pSeeable->CalcTotalName(false)
                                : MYSTR("(No Name)");
                            printf("\t\t(%d)> %s: %s\t\n", i, type, static_cast<const char*>(name));
                        }
                    }
                }
            }
        }
    }

    void ZVisibilitySpace::ShowSeeablePositions(ZDrawDebugRender* pRender)
    {
        ZCellCoordinate coord;
        for (coord.m_X = 0; coord.m_X < ZCellCoordinate::XSEGMENTS; ++coord.m_X)
        {
            for (coord.m_Z = 0; coord.m_Z < ZCellCoordinate::ZSEGMENTS; ++coord.m_Z)
            {
                for (coord.m_Y = 0; coord.m_Y < ZCellCoordinate::YSEGMENTS; ++coord.m_Y)
                {
                    ZSpaceCell& cell = GetCell(coord);
                    if (cell.GetNumSeeables() != 0)
                    {
                        for (uint8_t i = 0; i < cell.GetNumSeeables(); ++i)
                        {
                            const ZSeeable& seeable = ZCheckVisible::Instance().GetSeeable(cell.GetSeeableID(i));
                            printf("seeable %u at (%.2f, %.2f, %.2f)\n", cell.GetSeeableID(i), seeable.pos.x, seeable.pos.y, seeable.pos.z);
                        }
                    }
                }
            }
        }
    }

    void ZVisibilitySpace::ShowCells(ZDrawDebugRender* pRender)
    {
        ZCellCoordinate coord;
        for (coord.m_X = 0; coord.m_X < ZCellCoordinate::XSEGMENTS; ++coord.m_X)
        {
            for (coord.m_Z = 0; coord.m_Z < ZCellCoordinate::ZSEGMENTS; ++coord.m_Z)
            {
                for (coord.m_Y = 0; coord.m_Y < ZCellCoordinate::YSEGMENTS; ++coord.m_Y)
                {
                    ZVector3 vCellCenter;
                    GetCellPosition(vCellCenter, coord);
                    vCellCenter.x += m_vCellSize.x * 0.5f;
                    vCellCenter.y += m_vCellSize.y * 0.5f;
                    vCellCenter.z += m_vCellSize.z * 0.5f;
                    printf("cell (%d, %d, %d) center (%.2f, %.2f, %.2f), mode %d\n",
                        coord.m_X, coord.m_Y, coord.m_Z,
                        vCellCenter.x, vCellCenter.y, vCellCenter.z,
                        static_cast<int32_t>(g_iShowVisionCells));
                }
            }
        }
    }

    void ZVisibilitySpace::ShowCellsForSeer(ZDrawDebugRender* pRender)
    {
        const int32_t seerIndex = g_iShowCellsForSeer;
        if (seerIndex < 0 || seerIndex >= static_cast<int32_t>(Vision::MAXSEERS))
        {
            return;
        }
        ZCheckVisible& checkVisible = ZCheckVisible::Instance();
        SSeer& seer = checkVisible.m_Seers[seerIndex];
        if (!seer.pSeer)
        {
            return;
        }
        checkVisible.ShowConeForSeer(pRender, seer, static_cast<uint32_t>(seerIndex));
        ZCellCoordinate coord;
        for (coord.m_X = 0; coord.m_X < ZCellCoordinate::XSEGMENTS; ++coord.m_X)
        {
            for (coord.m_Z = 0; coord.m_Z < ZCellCoordinate::ZSEGMENTS; ++coord.m_Z)
            {
                coord.m_Y = 0;
                if (g_DebugShowCells.IsSet(coord))
                {
                    printf("cell (%d, %d, %d)\n", coord.m_X, coord.m_Y, coord.m_Z);
                }
            }
        }
    }

    ZCellCoordinate ZVisibilitySpace::FindCell(const float* vPos) const
    {
        return ZCellCoordinate(
            static_cast<int8_t>(static_cast<int>((vPos[0] - m_vLow.x) * m_vCellSizeInv.x)),
            static_cast<int8_t>(static_cast<int>((vPos[1] - m_vLow.y) * m_vCellSizeInv.y)),
            static_cast<int8_t>(static_cast<int>((vPos[2] - m_vLow.z) * m_vCellSizeInv.z)));
    }

    void ZVisibilitySpace::GetCellPosition(float* vPos, const ZCellCoordinate& coord) const
    {
        vPos[0] = static_cast<float>(coord.m_X) * m_vCellSize.x + m_vLow.x;
        vPos[1] = static_cast<float>(coord.m_Y) * m_vCellSize.y + m_vLow.y;
        vPos[2] = static_cast<float>(coord.m_Z) * m_vCellSize.z + m_vLow.z;
    }

    bool ZVisibilitySpace::CellIsWithinDistance(const ZCellCoordinate& coord, ZVector3& vPos, float fDistance)
    {
        ZVector3 vCellCenter;
        GetCellPosition(vCellCenter, coord);
        vCellCenter.x += m_vCellSize.x * 0.5f;
        vCellCenter.y += m_vCellSize.y * 0.5f;
        vCellCenter.z += m_vCellSize.z * 0.5f;

        const float dx = vPos.x - vCellCenter.x;
        const float dy = vPos.y - vCellCenter.y;
        const float dz = vPos.z - vCellCenter.z;

        return (dx * dx + dy * dy + dz * dz) <= fDistance * fDistance;
    }
}
