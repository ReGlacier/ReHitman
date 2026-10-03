#include <Glacier/Physics/ZCollisionWintel.h>
#include <Glacier/Physics/SExtendedImpactInfo.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <Glacier/Render/Prim/SPrimStrips.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZUniMemory.h>
#include <cmath>
#include <cstring>

namespace Glacier
{
    namespace
    {
        bool IntersectTriangle(const float* start, const float* direction, const float* a,
            const float* b, const float* c, bool twoSided, float& result)
        {
            const float e1[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] };
            const float e2[3] = { c[0] - a[0], c[1] - a[1], c[2] - a[2] };
            const float p[3] = { direction[1] * e2[2] - direction[2] * e2[1], direction[2] * e2[0] - direction[0] * e2[2], direction[0] * e2[1] - direction[1] * e2[0] };
            const float det = e1[0] * p[0] + e1[1] * p[1] + e1[2] * p[2];
            if (std::fabs(det) < 0.000001f || (!twoSided && det > 0.0f)) return false;
            const float invDet = 1.0f / det;
            const float tvec[3] = { start[0] - a[0], start[1] - a[1], start[2] - a[2] };
            const float u = (tvec[0] * p[0] + tvec[1] * p[1] + tvec[2] * p[2]) * invDet;
            if (u < 0.0f || u > 1.0f) return false;
            const float q[3] = { tvec[1] * e1[2] - tvec[2] * e1[1], tvec[2] * e1[0] - tvec[0] * e1[2], tvec[0] * e1[1] - tvec[1] * e1[0] };
            const float v = (direction[0] * q[0] + direction[1] * q[1] + direction[2] * q[2]) * invDet;
            if (v < 0.0f || u + v > 1.0f) return false;
            result = (e2[0] * q[0] + e2[1] * q[1] + e2[2] * q[2]) * invDet;
            return result >= 0.0f && result <= 1.0f;
        }

        const SPrimHeader* Prim(uint32_t id)
        {
            return ZPrimControlBase::GetPrimitive<const SPrimHeader>(id);
        }
    }

    ZCollisionWintel::~ZCollisionWintel() = default;

    bool ZCollisionWintel::CalcLineCollision(SExtendedImpactInfo* impact, uint32_t prim, const float* start, const float* direction, bool twoSided, uint32_t mask)
    {
        bool hit = false;
        while (prim)
        {
            const auto* header = Prim(prim);
            if (!header) break;
            const uint32_t next = *reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(header) + 8);
            if (header->lType == EPrimType::PTSTRIP || header->lType == EPrimType::PTDOT3STRIP)
            {
                const auto* strips = reinterpret_cast<const SPrimStripsWintel*>(header);
                if ((strips->lColiBits & mask) != 0)
                {
                    const auto* vertices = reinterpret_cast<const float*>(g_pRenderDll->m_pPrimBuffer + strips->lVertices);
                    const auto* indices = reinterpret_cast<const uint16_t*>(g_pRenderDll->m_pPrimBuffer + strips->lIndices);
                    const uint16_t stripCount = *indices++;
                    uint32_t triangleNr = 0;
                    for (uint16_t strip = 0; strip < stripCount; ++strip)
                    {
                        const uint16_t length = *indices++;
                        for (uint16_t i = 0; i + 2 < length; ++i)
                        {
                            const float* a = vertices + 9 * indices[i];
                            const float* b = vertices + 9 * indices[i + ((i & 1) != 0 ? 2 : 1)];
                            const float* c = vertices + 9 * indices[i + ((i & 1) != 0 ? 1 : 2)];
                            float t;
                            if (IntersectTriangle(start, direction, a, b, c, twoSided, t) && t < impact->fPercent)
                            {
                                impact->fPercent = t;
                                impact->vP1 = { a[0], a[1], a[2] };
                                impact->vP2 = { b[0], b[1], b[2] };
                                impact->vP3 = { c[0], c[1], c[2] };
                                impact->lTriangleNr = triangleNr;
                                hit = true;
                            }
                            ++triangleNr;
                        }
                        indices += length;
                    }
                }
            }
            else if (header->lType == EPrimType::PTOBJECTHEADER)
            {
                const auto* object = reinterpret_cast<const uint32_t*>(header);
                const uint32_t count = object[5];
                const auto* meshes = reinterpret_cast<const uint32_t*>(g_apPrimHandleToPointerTable[object[6]]);
                for (uint32_t i = 0; i < count; ++i)
                {
                    const auto* mesh = Prim(meshes[i]);
                    if (!mesh || mesh->lType != EPrimType::PTMESH ||
                        (*reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(mesh) + 20) & mask) == 0)
                        continue;

                    ZPrimHandle handle { meshes[i] };
                    auto* access = reinterpret_cast<ZPrimAccessMesh*>(ZPrimAccess::Create(handle));
                    if (!access) continue;
                    access->Lock(ZPrimAccess::LF_READONLY);
                    const uint32_t countTriangles = access->GetNumTriangles();
                    for (uint32_t first = 0; first < countTriangles; first += 32)
                    {
                        const uint32_t batch = (countTriangles - first > 32) ? 32 : countTriangles - first;
                        float triangles[288] {};
                        access->GetTriangles(first, batch, triangles);
                        for (uint32_t triangle = 0; triangle < batch; ++triangle)
                        {
                            float t;
                            if (IntersectTriangle(start, direction, triangles + triangle * 9,
                                triangles + triangle * 9 + 3, triangles + triangle * 9 + 6, twoSided, t) && t < impact->fPercent)
                            {
                                impact->fPercent = t;
                                impact->vP1 = triangles + triangle * 9;
                                impact->vP2 = triangles + triangle * 9 + 3;
                                impact->vP3 = triangles + triangle * 9 + 6;
                                impact->lTriangleNr = first + triangle;
                                hit = true;
                            }
                        }
                    }
                    access->Unlock();
                    access->Destroy();
                }
            }
            prim = next;
        }
        if (hit)
        {
            vaddscalar(impact->vPosition, start, direction, impact->fPercent);
            impact->m_HitCache.___u0.__s0.lIdLo = 0;
        }
        return hit;
    }

    bool ZCollisionWintel::CalcLineCollision(SExtendedImpactInfo* impact, const STempStripsUniqueId* id, const float* start, const float* direction, bool twoSided, uint32_t mask)
    {
        if (!CheckLineCollision(id, start, direction, twoSided, mask)) return false;
        impact->m_HitCache = *id;
        return true;
    }

    bool ZCollisionWintel::CheckSphereCollision(uint32_t prim, uint8_t mask, const float* position, const float* matrix, const float* dimensions)
    {
        float sphereMatrix[9] = { matrix[0] / dimensions[2], matrix[1] / dimensions[2], matrix[2] / dimensions[2], matrix[3] / dimensions[1], matrix[4] / dimensions[1], matrix[5] / dimensions[1], matrix[6] / dimensions[0], matrix[7] / dimensions[0], matrix[8] / dimensions[0] };
        while (prim)
        {
            const auto* strips = reinterpret_cast<const SPrimStripsWintel*>(Prim(prim));
            if (!strips) return false;
            const uint32_t next = strips->lNextPrim;
            if (strips->lType == EPrimType::PTSTRIP && (strips->lColiBits & mask) != 0)
            {
                const auto* vertices = reinterpret_cast<const float*>(g_pRenderDll->m_pPrimBuffer + strips->lVertices);
                const auto* indices = reinterpret_cast<const uint16_t*>(g_pRenderDll->m_pPrimBuffer + strips->lIndices);
                const uint16_t count = *indices++;
                for (uint16_t s = 0; s < count; ++s)
                {
                    const uint16_t length = *indices++;
                    for (uint16_t i = 0; i + 2 < length; ++i)
                    {
                        const float* a = vertices + 9 * indices[i];
                        const float* b = vertices + 9 * indices[i + ((i & 1) != 0 ? 2 : 1)];
                        const float* c = vertices + 9 * indices[i + ((i & 1) != 0 ? 1 : 2)];
                        if (ZCommonAlgorithms::PolySphColl(position, sphereMatrix, a, b, c)) return true;
                    }
                    indices += length;
                }
            }
            prim = next;
        }
        return false;
    }

    void ZCollisionWintel::CreateSubStripId(STempStripsUniqueId* dst, const STempStripsUniqueId* src, const uint32_t count)
    {
        dst->___u0.__s0.lIdLo = count | src->___u0.__s0.lIdLo;
        dst->___u0.__s0.lIdHi = src->___u0.__s0.lIdHi;
    }

    STempStrips* ZCollisionWintel::GetStripsFromPrim(const uint32_t primId)
    {
        const auto* header = Prim(primId);
        if (!header || header->lType != EPrimType::PTSTRIP) return nullptr;

        STempStrips* result = nullptr;
        uint32_t currentPrim = primId;
        while (currentPrim)
        {
            const auto* strips = reinterpret_cast<const SPrimStripsWintel*>(Prim(currentPrim));
            if (!strips) break;

            const auto* lengths = reinterpret_cast<const uint16_t*>(g_pRenderDll->m_pPrimBuffer + strips->lIndices);
            const uint16_t stripCount = lengths[0];
            uint32_t vertexCount = 0;
            const uint16_t* length = lengths + 1;
            for (uint16_t i = 0; i < stripCount; ++i)
            {
                vertexCount += length[0];
                length += length[0] + 1;
            }

            auto* data = ZUniMemory::New<STempStrips>();
            std::memset(data, 0, sizeof(STempStrips));
            data->pNext = result;
            data->lNrEntries = vertexCount;
            data->lNrStrips = stripCount;
            data->pVertices = static_cast<float*>(ZUniMemory::Allocate(vertexCount * sizeof(ZVector3)));
            data->pLengths = static_cast<uint32_t*>(ZUniMemory::Allocate(stripCount * sizeof(uint32_t)));
            data->pUniqueId = static_cast<STempStripsUniqueId*>(ZUniMemory::Allocate(vertexCount * sizeof(STempStripsUniqueId)));

            const auto* vertices = reinterpret_cast<const float*>(g_pRenderDll->m_pPrimBuffer + strips->lVertices);
            const uint16_t* indices = lengths + 1;
            uint32_t outputIndex = 0;
            for (uint16_t strip = 0; strip < stripCount; ++strip)
            {
                const uint16_t stripLength = indices[0];
                data->pLengths[strip] = stripLength;
                ++indices;
                for (uint16_t vertex = 0; vertex < stripLength; ++vertex, ++outputIndex)
                {
                    const float* source = vertices + 9 * indices[vertex];
                    std::memcpy(data->pVertices + outputIndex * 3, source, sizeof(ZVector3));
                    data->pUniqueId[outputIndex].___u0.__s0.lIdHi = currentPrim;
                    data->pUniqueId[outputIndex].___u0.__s0.lIdLo =
                        ((reinterpret_cast<const char*>(indices) - reinterpret_cast<const char*>(lengths)) << 15) |
                        (static_cast<uint32_t>(vertex) << 31);
                }
                indices += stripLength;
            }
            result = data;
            currentPrim = strips->lNextPrim;
        }
        return result;
    }

    uint32_t ZCollisionWintel::GetPrimFromStripUniqueId(const STempStripsUniqueId* id) { return id->___u0.__s0.lIdHi; }

    uint32_t ZCollisionWintel::GetColiBits(const STempStripsUniqueId* id)
    {
        return *reinterpret_cast<const uint32_t*>(&g_pRenderDll->m_pPrimBuffer[id->___u0.__s0.lIdHi + 0x34]);
    }

    char* ZCollisionWintel::GetStripVerticesFromId(ZRawStrip* raw, const STempStripsUniqueId* id, uint32_t mask, const float* m0, const float* p0, const float* s0)
    {
        const auto* strips = reinterpret_cast<const SPrimStripsWintel*>(Prim(id->___u0.__s0.lIdHi));
        if (!strips || (strips->lColiBits & mask) == 0) return reinterpret_cast<char*>(raw);
        const uint32_t entry = (id->___u0.__s0.lIdLo >> 16) & 0x7FFFu;
        const uint32_t length = id->___u0.__s0.lIdLo & 0xFFFFu;
        GetStripVerticesStrip(raw, strips, id, m0, p0, s0, (id->___u0.__s0.lIdLo & 0x80000000u) != 0, entry, length);
        return reinterpret_cast<char*>(raw + 1) + (raw->m_lLength & 0x7FFFFFFFu) * sizeof(ZVector3);
    }

    bool ZCollisionWintel::CheckLineCollision(const STempStripsUniqueId* id, const float* start, const float* direction, bool twoSided, uint32_t mask)
    {
        const auto* strips = reinterpret_cast<const SPrimStripsWintel*>(Prim(id->___u0.__s0.lIdHi));
        if (!strips || strips->lType != EPrimType::PTSTRIP || (strips->lColiBits & mask) == 0) return false;
        const auto* vertices = reinterpret_cast<const float*>(g_pRenderDll->m_pPrimBuffer + strips->lVertices);
        const auto* indices = reinterpret_cast<const uint16_t*>(g_pRenderDll->m_pPrimBuffer + strips->lIndices);
        const uint32_t first = (id->___u0.__s0.lIdLo >> 16) & 0x7FFFu;
        const uint32_t end = first + (id->___u0.__s0.lIdLo & 0xFFFFu) - 2;
        for (uint32_t i = first; i < end; ++i)
        {
            float t;
            if (IntersectTriangle(start, direction, vertices + 9 * indices[i], vertices + 9 * indices[i + ((i & 1) != 0 ? 2 : 1)], vertices + 9 * indices[i + ((i & 1) != 0 ? 1 : 2)], twoSided, t)) return true;
        }
        return false;
    }

    void ZCollisionWintel::GetStripVerticesStrip(ZRawStrip* raw, const SPrimStripsWintel* strips, const STempStripsUniqueId* id, const float* m0, const float* p0, const float*, bool, uint32_t entry, uint32_t length)
    {
        const auto* vertices = reinterpret_cast<const float*>(g_pRenderDll->m_pPrimBuffer + strips->lVertices);
        const auto* indices = reinterpret_cast<const uint16_t*>(g_pRenderDll->m_pPrimBuffer + strips->lIndices) + entry;
        float* output = reinterpret_cast<float*>(raw + 1);
        for (uint32_t i = 0; i < length; ++i, output += 3)
        {
            vsub(output, vertices + 9 * indices[i], p0);
            vmtmul(output, m0);
        }
        raw->m_HitCache = *id;
        raw->m_lLength = length;
        raw->m_lTriangleStartNr = 0;
    }

    void ZCollisionWintel::GetStripVerticesObject(ZRawStrip*, SPrimObjectHeader*, const STempStripsUniqueId*, const float*, const float*, const float*)
    {
        ZASSERT(false);
    }

    void ZCollisionWintel::GetStripVerticesMesh(ZRawStrip* raw, uint32_t entry, ZPrimAccessMesh* access, const STempStripsUniqueId* id, const float* m0, const float* p0, const float*)
    {
        const uint32_t count = access->GetNumTriangles() - entry;
        const uint32_t batch = count > 32 ? 32 : count;
        float* output = reinterpret_cast<float*>(raw + 1);
        access->Lock(ZPrimAccess::LF_READONLY);
        access->GetTriangles(entry, batch, output);
        access->Unlock();
        for (uint32_t i = 0; i < batch * 3; ++i)
        {
            vsub(output + i * 3, output + i * 3, p0);
            vmtmul(output + i * 3, m0);
        }
        raw->m_HitCache = *id;
        raw->m_lLength = batch | 0x80000000u;
    }

    ZCollisionBase* ZCollisionBase::InitCollision(bool bPackEnabled)
    {
        ZASSERT(!bPackEnabled);
        auto* instance = ZUniMemory::New<ZCollisionWintel>(bPackEnabled);
        ZCollisionBase::s_pCollisionBase = instance;
        return instance;
    }
}
