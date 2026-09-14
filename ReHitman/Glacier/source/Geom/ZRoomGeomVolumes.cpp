#include <Glacier/Geom/ZRoomGeomVolumes.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZBaseGeomRoomList.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Geom/ZENVIRONMENT.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/ZSTL/StringUtils.h>


namespace Glacier
{
    namespace
    {
        // KL2 0x822F5D70: append pRoomEnvironment to pEnvironments if not already present.
        uint32_t AddEnvironmentToList(ZBaseGeom* pRoomEnvironment, ZBaseGeom** pEnvironments, uint32_t lNumEnvironments)
        {
            for (uint32_t i = 0; i < lNumEnvironments; ++i)
            {
                if (pEnvironments[i] == pRoomEnvironment)
                {
                    return lNumEnvironments;
                }
            }

            pEnvironments[lNumEnvironments] = pRoomEnvironment;
            return lNumEnvironments + 1;
        }
    }

    // PC 0047B860: "character environment" id = the base geom of the environment's
    // character light (m_pCharacterLight->m_baseGeom). The decompiler rendered this as
    // `m_pExtraGeom[2].vftable->PreSave` but it is just two pointer reads.
    ZBaseGeom* GetCharacterEnvironment(ZBaseGeom* pEnvironment)
    {
        ZENVIRONMENT* pEnvGeom = static_cast<ZENVIRONMENT*>(pEnvironment->m_pExtraGeom);
        if (pEnvGeom->m_pCharacterLight)
        {
            return pEnvGeom->m_pCharacterLight->m_baseGeom;
        }

        return nullptr;
    }

    void* ZRoomGeomVolumes::ZVolumesBlock::Alloc(uint32_t lByteSize)
    {
        ZASSERT(lByteSize <= BLOCK_SIZE);

        char* pCurrent = m_pCurrent;
        if (pCurrent + lByteSize > reinterpret_cast<char*>(&m_pCurrent))
        {
            return nullptr;
        }

        m_pCurrent = pCurrent + lByteSize;
        return pCurrent;
    }

    ZRoomGeomVolumes::ZRoomGeomVolumes() = default;

    ZRoomGeomVolumes::~ZRoomGeomVolumes()
    {
        while (m_pFirstBlock != &m_DefaultBlock)
        {
            ZVolumesBlock* pNext = m_pFirstBlock->m_pNext;
            ZUniMemory::Free(m_pFirstBlock);
            m_pFirstBlock = pNext;
        }

        for (uint32_t i = 0; i < m_Rooms.m_lNrEntries; ++i)
        {
            m_Rooms.m_Array[i]->m_pRoomCache = nullptr;
            m_Rooms.m_Array[i]->m_lCacheIndex = -1;
        }

        m_Rooms.m_lNrEntries = 0;
    }

    void* ZRoomGeomVolumes::Alloc(uint32_t lByteSize)
    {
        void* pResult = m_pFirstBlock->Alloc(lByteSize);
        while (pResult == nullptr)
        {
            ZVolumesBlock* pNewBlock = static_cast<ZVolumesBlock*>(ZUniMemory::Allocate(ZVolumesBlock::BLOCK_SIZE + 0x14));
            if (pNewBlock)
            {
                pNewBlock->m_pNext = nullptr;
                pNewBlock->m_pCurrent = pNewBlock->m_Memory;
            }

            pNewBlock->m_pNext = m_pFirstBlock;
            m_pFirstBlock = pNewBlock;

            pResult = pNewBlock->Alloc(lByteSize);
        }

        return pResult;
    }

    ZRoomGeomVolumes::ZHeader* ZRoomGeomVolumes::CreateGroupVolumes(ZGROUP* pGroup)
    {
        ZBaseGeom* pBaseGeoms[4096];

        const uint32_t lNumGeoms = GetGroupGeomsEnvironmentsRecursive(pGroup, pBaseGeoms, nullptr, nullptr, nullptr);

        ZHeader* pHeader = static_cast<ZHeader*>(Alloc(84 * lNumGeoms + 12));
        if (!pHeader)
        {
            return nullptr;
        }

        ZBaseGeomVolume* pVolume = reinterpret_cast<ZBaseGeomVolume*>(pHeader + 1);

        for (uint32_t i = 0; i < lNumGeoms; ++i)
        {
            ZBaseGeom* pGeom = pBaseGeoms[i];

            pGeom->GetRootTM(pVolume->m_RootPosition.m0, pVolume->m_RootPosition.p0);
            vmmul(pVolume->m_vSize.Get(), pGeom->m_vCen.Get(), pVolume->m_RootPosition.m0.Get());
            vadd(pVolume->m_vSize.Get(), pVolume->m_RootPosition.p0.Get());
            pVolume->m_fRadius = pGeom->m_fRadius;
            pVolume->m_vCenter = pGeom->m_vSize;
            pVolume->m_pBaseGeom = pGeom;
            pVolume->m_pBaseGeomEnvironment = nullptr;

            ++pVolume;
        }

        pHeader->m_lNumGeoms = lNumGeoms;
        pHeader->m_lNumLights = 0;
        pHeader->m_lNumEnvironments = 0;

        return pHeader;
    }

    ZRoomGeomVolumes::ZHeader* ZRoomGeomVolumes::GetVolumesHeader(ZROOM* pRoom, bool bExitsEnabled)
    {
        // Cache the room's volumes keyed by its cache index (see m_Rooms / m_RoomCacheIndexRemap usage).
        if (pRoom->m_lCacheIndex < 0)
        {
            return nullptr;
        }

        ZBaseGeom* pBaseGeoms[4096];
        ZBaseGeom* pBaseGeomEnvironments[4096];
        uint32_t lNumLights = 0;
        uint32_t lNumEnvironments = 0;

        const uint32_t lNumGeoms = GetGeomsInRoom(&lNumLights, &lNumEnvironments, pBaseGeoms, pBaseGeomEnvironments, pRoom, bExitsEnabled);

        ZHeader* pHeader = static_cast<ZHeader*>(Alloc(84 * lNumGeoms + 12));
        if (!pHeader)
        {
            return nullptr;
        }

        ZBaseGeomVolume* pVolume = reinterpret_cast<ZBaseGeomVolume*>(pHeader + 1);

        for (uint32_t i = 0; i < lNumGeoms; ++i)
        {
            ZBaseGeom* pGeom = pBaseGeoms[i];

            pGeom->GetRootTM(pVolume->m_RootPosition.m0, pVolume->m_RootPosition.p0);
            vmmul(pVolume->m_vSize.Get(), pGeom->m_vCen.Get(), pVolume->m_RootPosition.m0.Get());
            vadd(pVolume->m_vSize.Get(), pVolume->m_RootPosition.p0.Get());
            pVolume->m_fRadius = pGeom->m_fRadius;
            pVolume->m_vCenter = pGeom->m_vSize;
            pVolume->m_pBaseGeom = pGeom;
            pVolume->m_pBaseGeomEnvironment = pBaseGeomEnvironments ? pBaseGeomEnvironments[i] : nullptr;

            ++pVolume;
        }

        pHeader->m_lNumGeoms = lNumGeoms;
        pHeader->m_lNumLights = lNumLights;
        pHeader->m_lNumEnvironments = lNumEnvironments;

        return pHeader;
    }

    uint32_t ZRoomGeomVolumes::GetGroupGeomsEnvironmentsRecursive(ZGROUP* pGroup, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, ZBaseGeom* pRoomEnvironment, ZBaseGeom* pRoomCharacterEnvironment)
    {
        uint32_t lNum = 0;

        // Collect the group's own (non-group) children, walking backwards.
        ZBaseGeom* pGeom = pGroup->m_pGroupLast;
        if (ForNotGroupsCheck(pGeom))
        {
            ZBaseGeom** pEnv = pBaseGeomEnvironments;
            do
            {
                if (!pGeom->IsDerivedFrom<ZENVIRONMENT>()
                    && (pGeom->m_lControl & 0x202C00) == 0
                    && (pGeom->IsDerivedFrom<ZSTDOBJ>() || pGeom->IsDerivedFrom<ZLIGHT>()))
                {
                    if (pBaseGeomEnvironments)
                    {
                        *pEnv = pGeom->IsDerivedFrom<ZLNKOBJ>() ? pRoomCharacterEnvironment : pRoomEnvironment;
                    }

                    pBaseGeoms[lNum] = pGeom;
                    ++lNum;
                    ++pEnv;
                }

                pGeom = pGeom->GetPrev();
            }
            while (ForNotGroupsCheck(pGeom));
        }

        // Recurse into sub-groups, skipping rooms.
        ZBaseGeom* pChild = pGroup->m_pGroupFirst;
        while (ForGroupsCheck(pChild))
        {
            if (!pChild->IsDerivedFrom<ZROOM>())
            {
                ZGROUP* pChildGroup = static_cast<ZGROUP*>(pChild->m_pExtraGeom);

                // Locate the environment this group belongs to (walk the sibling
                // chain backwards until a non-"*_env_character*" environment).
                ZBaseGeom* pEnvGeom = pChildGroup->m_pGroupLast;
                if (ForNotGroupsCheck(pEnvGeom))
                {
                    bool bFound = false;
                    while (ForNotGroupsCheck(pEnvGeom))
                    {
                        if (pEnvGeom->IsDerivedFrom<ZENVIRONMENT>())
                        {
                            const char* pName = pEnvGeom->m_Name;
                            if (!pName)
                            {
                                pName = "<NONAME>";
                            }

                            if (striwcmp(pName, "*_env_character*"))
                            {
                                bFound = true;
                                break;
                            }
                        }

                        pEnvGeom = pEnvGeom->GetPrev();
                    }

                    if (bFound)
                    {
                        pRoomEnvironment = pEnvGeom;
                        pRoomCharacterEnvironment = GetCharacterEnvironment(pEnvGeom);
                    }
                }

                if (!pChild->IsDerivedFrom<ZENVIRONMENT>()
                    && (pChild->m_lControl & 0x202C00) == 0
                    && (pChildGroup->m_lGroupCon & 0x400) == 0)
                {
                    ZBaseGeom** pEnv = pBaseGeomEnvironments ? &pBaseGeomEnvironments[lNum] : nullptr;
                    lNum += GetGroupGeomsEnvironmentsRecursive(pChildGroup, &pBaseGeoms[lNum], pEnv, pRoomEnvironment, pRoomCharacterEnvironment);
                }
            }

            pChild = pChild->Next();
        }

        return lNum;
    }

    uint32_t ZRoomGeomVolumes::GetGeomsInRoom(uint32_t* pNumLights, uint32_t* pNumEnvironments, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, ZROOM* pRoom, bool bExitsEnabled)
    {
        ZBaseGeom* pEnvironment = pRoom->m_pEnvironment;
        ZBaseGeom* pEnvironments[72];
        uint32_t lNumEnvironments = 0;
        ZBaseGeom* pCharEnv = nullptr;

        if (pEnvironment)
        {
            lNumEnvironments = AddEnvironmentToList(pEnvironment, pEnvironments, 0);
            pCharEnv = GetCharacterEnvironment(pEnvironment);
            if (pCharEnv)
            {
                lNumEnvironments = AddEnvironmentToList(pCharEnv, pEnvironments, lNumEnvironments);
            }
            else
            {
                pCharEnv = pEnvironment;
            }
        }

        uint32_t lNumGeoms;
        if (bExitsEnabled)
        {
            ZBaseGeom** pListEnd = pBaseGeoms + 0x1000;

            const uint32_t lDrawGeomCount = static_cast<uint32_t>(pRoom->GetDynamicDrawGeomsLists(pBaseGeoms, pListEnd) - pBaseGeoms);
            const uint32_t lTotal = static_cast<uint32_t>(pRoom->GetDynamicLightsInRoom(pBaseGeoms + lDrawGeomCount, pListEnd) - pBaseGeoms);
            lNumGeoms = 0;

            for (uint32_t i = 0; i < lTotal; ++i)
            {
                ZBaseGeom* pGeom = pBaseGeoms[i];
                if (pGeom->IsDerivedFrom<ZENVIRONMENT>() || (pGeom->m_lControl & 0x202C00) != 0)
                {
                    continue;
                }

                ZBaseGeom* pEnv = nullptr;
                if (pGeom->IsDerivedFrom<ZLNKOBJ>())
                {
                    pEnv = GetCharacterEnvironment(pEnvironment);

                    ZBaseGeom* pParent = pGeom->m_pParent;
                    if ((pParent->m_lControl & 0x40000) != 0 && (pParent->m_lControl & 0x40000000) == 0)
                    {
                        ZBaseGeomRoomList* pRoomList = pParent->GetRoomList();
                        if (pRoomList && pRoomList->m_cNrRooms)
                        {
                            ZBaseGeom* pRoomEnv = pRoomList->m_pRooms[0]->m_pEnvironment;
                            if (pRoomEnv)
                            {
                                pEnv = GetCharacterEnvironment(pRoomEnv);
                            }
                        }
                    }
                }
                else if ((pGeom->m_lControl & 0x40000000) != 0)
                {
                    pEnv = pGeom->GetOwnerRoom()->m_pEnvironment;
                }
                else
                {
                    pEnv = pEnvironment;
                }

                pBaseGeomEnvironments[lNumGeoms] = pEnv;
                pBaseGeoms[lNumGeoms] = pGeom;
                ++lNumGeoms;
            }

            for (uint32_t i = 0; i < pRoom->m_lNrNeighborRooms; ++i)
            {
                const ZROOM::ZNeighborRoom& neighbor = pRoom->m_pNeighborRooms[i];
                ZBaseGeom* pNeighborEnv = neighbor.m_pNeighbor->m_pEnvironment;
                if (pNeighborEnv)
                {
                    lNumEnvironments = AddEnvironmentToList(pNeighborEnv, pEnvironments, lNumEnvironments);
                }

                for (uint32_t j = 0; j < neighbor.m_lNrGeomsInExit; ++j)
                {
                    ZBaseGeom* pGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(neighbor.m_pGeomsInExit[j]);
                    if (pGeom && !pGeom->IsDerivedFrom<ZENVIRONMENT>() && (pGeom->m_lControl & 0x202C00) == 0)
                    {
                        pBaseGeomEnvironments[lNumGeoms] = pNeighborEnv;
                        pBaseGeoms[lNumGeoms] = pGeom;
                        ++lNumGeoms;
                    }
                }
            }
        }
        else
        {
            lNumGeoms = GetGroupGeomsEnvironmentsRecursive(pRoom, pBaseGeoms, pBaseGeomEnvironments, pEnvironment, nullptr);
        }

        // Reorder lights to the back (swap with the tail on each light).
        uint32_t lNumNonLights = lNumGeoms;
        for (uint32_t i = 0; i < lNumNonLights;)
        {
            ZBaseGeom* pGeom = pBaseGeoms[i];
            if (pGeom->IsDerivedFrom<ZLIGHT>())
            {
                --lNumNonLights;

                ZBaseGeom* pBackGeom = pBaseGeoms[lNumNonLights];
                pBaseGeoms[lNumNonLights] = pGeom;
                pBaseGeoms[i] = pBackGeom;

                ZBaseGeom* pBackEnv = pBaseGeomEnvironments[lNumNonLights];
                pBaseGeomEnvironments[lNumNonLights] = pBaseGeomEnvironments[i];
                pBaseGeomEnvironments[i] = pBackEnv;
            }
            else
            {
                ++i;
            }
        }

        const uint32_t lNumLights = lNumGeoms - lNumNonLights;

        *pNumEnvironments = lNumEnvironments;

        for (uint32_t i = 0; i < lNumEnvironments; ++i)
        {
            pBaseGeoms[lNumGeoms] = pEnvironments[i];
            pBaseGeomEnvironments[lNumGeoms] = nullptr;
            ++lNumGeoms;
        }

        *pNumLights = lNumLights + lNumEnvironments;

        return lNumGeoms;
    }
}
