#include <Glacier/Fysix/ZVertexExtend.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZVertexExtend::ZVertexExtend()
        : ZDynamicsExtend()
    {
        // Do nothing
    }

    ZVertexExtend::~ZVertexExtend()
    {
        DestroyMapper();
    }

    bool ZVertexExtend::Update(uint16_t body, ZGEOM* geom)
    {
        ZASSERT(m_pMapper);

        if (!m_pProps[body].active)
        {
            return false;
        }

        auto* pBody = m_kConSys.GetBody(body);
        if (!m_wMappings)
        {
            return true;
        }

        const uint32_t lPrimID = geom->Prim();

        // m_pMapper points to array of SVertexMapper (12 bytes each):
        // +0x00: vtbl
        // +0x04: uint16 vertexCount
        // +0x08: uint16* indices
        const uint8_t* pMapperData = reinterpret_cast<const uint8_t*>(m_pMapper);

        for (uint16_t mapIdx = 0; mapIdx < m_wMappings; ++mapIdx)
        {
            const uint32_t lSubPrim = g_pRenderDll->m_pPrimControl->GetSubPrim(lPrimID, mapIdx);
            if (!lSubPrim)
            {
                break;
            }

            ZPrimHandle hPrim;
            hPrim.m_lHandleValue = lSubPrim;

            auto* pPrim = static_cast<ZPrimAccessMesh*>(ZPrimAccess::Create(hPrim));
            if (!pPrim)
            {
                return false;
            }

            pPrim->Lock(ZPrimAccess::LF_WRITEONLY);

            // Access mapper entry: 12 bytes stride
            const uint16_t lVertexCount = *reinterpret_cast<const uint16_t*>(pMapperData + 4);
            const uint16_t* pIndices = *reinterpret_cast<const uint16_t* const*>(pMapperData + 8);

            if (pPrim->GetNumVertices() != lVertexCount)
            {
                pPrim->Unlock();
                pPrim->Destroy();
                return false;
            }

            for (uint32_t vertIdx = 0; vertIdx < lVertexCount; ++vertIdx)
            {
                float vPos[3];
                pBody->GetParticlePos(*reinterpret_cast<float(*)[3]>(vPos), pIndices[vertIdx]);
                pPrim->SetPositions(vertIdx, 1, vPos);
            }

            pPrim->Unlock();
            pPrim->Destroy();

            pMapperData += 12;
        }

        return true;
    }

    bool ZVertexExtend::CreateMapper(const SGenericMapper* mpr, uint16_t mappings)
    {
        m_wMappings = mappings;

        // Allocate array with count header: [count][SVertexMapper x mappings]
        const uint32_t lAllocSize = sizeof(uint32_t) + sizeof(SVertexMapper) * mappings;
        void* pRaw = ZUniMemory::Allocate(lAllocSize);
        if (!pRaw)
        {
            m_pMapper = nullptr;
            return false;
        }

        // Store count at header
        *static_cast<uint32_t*>(pRaw) = mappings;

        // Construct array of SVertexMapper
        SVertexMapper* pMappers = reinterpret_cast<SVertexMapper*>(static_cast<uint8_t*>(pRaw) + sizeof(uint32_t));
        for (uint16_t i = 0; i < mappings; ++i)
        {
            new (&pMappers[i]) SVertexMapper();
        }

        m_pMapper = pMappers;

        if (!mappings)
        {
            return true;
        }

        // Copy data from source mapper
        const auto* pSrc = reinterpret_cast<const SVertexMapper*>(mpr);
        auto* pDst = pMappers;

        for (uint16_t i = 0; i < mappings; ++i)
        {
            const uint16_t wCount = pSrc[i].m_wVertexCount;
            pDst[i].m_wVertexCount = wCount;

            if (wCount)
            {
                pDst[i].m_pIndices = static_cast<uint16_t*>(ZUniMemory::Allocate(2 * wCount));
                for (uint16_t j = 0; j < wCount; ++j)
                {
                    pDst[i].m_pIndices[j] = pSrc[i].m_pIndices[j];
                }
            }
            else
            {
                pDst[i].m_pIndices = nullptr;
            }
        }

        return true;
    }

    void ZVertexExtend::DestroyMapper()
    {
        if (m_pMapper)
        {
            auto* pMappers = static_cast<SVertexMapper*>(m_pMapper);
            const uint32_t lCount = *reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(m_pMapper) - sizeof(uint32_t));

            // Destruct in reverse order
            for (int32_t i = static_cast<int32_t>(lCount) - 1; i >= 0; --i)
            {
                pMappers[i].~SVertexMapper();
            }

            ZUniMemory::Free(reinterpret_cast<uint8_t*>(m_pMapper) - sizeof(uint32_t));
            m_pMapper = nullptr;
        }
    }

    const REFTAB* ZVertexExtend::GetCollisionFaces(uint16_t body, ZGEOM* geom)
    {
        return nullptr;
    }

    bool ZVertexExtend::Collision(uint16_t body, ZGEOM* geom)
    {
        return false;
    }

    bool ZVertexExtend::Fracture(uint16_t body, ZGEOM* geom)
    {
        return false;
    }
}