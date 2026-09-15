#include <Glacier/Render/Object/ZRenderObjectInstanceSpritesArrayD3D.h>
#include <Glacier/Render/Entry/ZRenderEntrySprite.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Prim/SPrimSpritesArray.h>
#include <Glacier/Render/Sprite/SSpriteArray.h>
#include <Glacier/Render/Sprite/SSpriteArrayElement.h>
#include <Glacier/Render/Sprite/SSpriteArrayElementRaw.h>
#include <Glacier/Render/Sprite/SSpriteArrayElementUV.h>
#include <Glacier/Render/Sprite/SSpriteArrayElementParticle.h>
#include <Glacier/Render/Sprite/SSpriteArrayBox.h>
#include <Glacier/Render/Sprite/SPRITETYPE.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRD3DDynamicVB.h>
#include <Glacier/Render/ZRD3DDynamicIB.h>
#include <Glacier/Render/Prim/SVertexWintel.h>
#include <cmath>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t kQuadVertices = 4;
        constexpr uint32_t kQuadIndices = 6;

        void SetQuadVertex(SVertexWintel& rVertex, const ZVector3& position, float u, float v, uint32_t color)
        {
            rVertex.p = position;
            rVertex.n = ZVector3(0.0f, 1.0f, 0.0f);
            rVertex.c = color;
            rVertex.t = ZVector2(u, v);
        }

        ZVector3 RotatePoint(float x, float z, float angle)
        {
            const float c = std::cos(angle);
            const float s = std::sin(angle);
            return ZVector3(x * c - z * s, 0.0f, x * s + z * c);
        }

        void WriteQuad(SVertexWintel* pVertices, uint16_t* pIndices, uint32_t lVertex, uint32_t lIndex,
            const ZVector3& position, float fWidth, float fHeight, float fAngle,
            const float* pUV, const uint32_t* pColors)
        {
            const float x = fWidth * 0.5f;
            const float z = fHeight * 0.5f;
            const ZVector3 corners[4]
            {
                RotatePoint(-x, -z, fAngle), RotatePoint(-x, z, fAngle),
                RotatePoint(x, z, fAngle), RotatePoint(x, -z, fAngle)
            };
            const float defaultUV[8] { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
            const float* uv = pUV ? pUV : defaultUV;
            for (uint32_t i = 0; i < 4; ++i)
            {
                const uint32_t color = pColors ? pColors[i] : 0xFFFFFFFFu;
                SetQuadVertex(pVertices[lVertex + i], position + corners[i], uv[i * 2], uv[i * 2 + 1], color);
            }
            pIndices[lIndex + 0] = static_cast<uint16_t>(lVertex + 0);
            pIndices[lIndex + 1] = static_cast<uint16_t>(lVertex + 1);
            pIndices[lIndex + 2] = static_cast<uint16_t>(lVertex + 2);
            pIndices[lIndex + 3] = static_cast<uint16_t>(lVertex + 0);
            pIndices[lIndex + 4] = static_cast<uint16_t>(lVertex + 2);
            pIndices[lIndex + 5] = static_cast<uint16_t>(lVertex + 3);
        }
    }

    void DrawSpriteArrayD3D(ZRenderObjectInstanceD3D* pInstance, ZRenderContext* pContext)
    {
        if (!pInstance || !pContext || !pInstance->m_pRenderEntry || !pInstance->m_pRenderObject)
            return;

        const auto* pEntry = reinterpret_cast<const ZRenderEntrySprite*>(pInstance->m_pRenderEntry);
        const SSpriteArray* pArrays = pEntry->m_pSpriteArray;
        const auto* pPrim = pInstance->m_pRenderObject->m_hPrim.Get<SPrimSpritesArray>();
        if (!pArrays || !pPrim || !pArrays->lNumSprites)
            return;

        if (pPrim->lSpriteType != SPRITETYPE_ARRAY &&
            pPrim->lSpriteType != SPRITETYPE_ARRAY_UV &&
            pPrim->lSpriteType != SPRITETYPE_ARRAY_RAW &&
            pPrim->lSpriteType != SPRITETYPE_ARRAY_PARTICLE &&
            pPrim->lSpriteType != SPRITETYPE_ARRAY_BOX &&
            pPrim->lSpriteType != SPRITETYPE_ARRAY_BOX_SCALE)
        {
            return;
        }

        auto* pDynamicVB = ZSharedResourcesD3D::g_pInstance->m_pDVB;
        auto* pDynamicIB = ZSharedResourcesD3D::g_pInstance->m_pDIB;
        const auto* pBoxArray = reinterpret_cast<const SSpriteArrayBox*>(pArrays);
        const uint32_t lQuadCount = (pPrim->lSpriteType == SPRITETYPE_ARRAY_BOX ||
            pPrim->lSpriteType == SPRITETYPE_ARRAY_BOX_SCALE) ? pBoxArray->lNrBoxes : pArrays->lNumSprites;
        const uint32_t lVertexCount = lQuadCount * kQuadVertices;
        const uint32_t lIndexCount = lQuadCount * kQuadIndices;
        uint32_t lVertexStart = 0;
        uint32_t lIndexStart = 0;
        auto* pVertices = static_cast<SVertexWintel*>(pDynamicVB->Lock(lVertexCount, sizeof(SVertexWintel), lVertexStart));
        auto* pIndices = static_cast<uint16_t*>(pDynamicIB->Lock(lIndexCount, lIndexStart));
        if (!pVertices || !pIndices)
        {
            if (pVertices)
                pDynamicVB->Unlock();
            if (pIndices)
                pDynamicIB->Unlock();
            return;
        }

        for (uint32_t i = 0; i < lQuadCount; ++i)
        {
            const uint32_t lVertex = i * kQuadVertices;
            const uint32_t lIndex = i * kQuadIndices;
            if (pPrim->lSpriteType == SPRITETYPE_ARRAY_PARTICLE)
            {
                const auto& sprite = pArrays->pSpritesParticle[i];
                const float fAge = sprite.fEndTime - sprite.fStartTime;
                const float fScale = sprite.fStartScale + fAge * sprite.fAgeDepthBias;
                WriteQuad(pVertices, pIndices, lVertex, lIndex, sprite.vC1,
                    fScale, fScale, sprite.fStartAngle, nullptr, nullptr);
            }
            else if (pPrim->lSpriteType == SPRITETYPE_ARRAY_BOX ||
                pPrim->lSpriteType == SPRITETYPE_ARRAY_BOX_SCALE)
            {
                const SSpriteBox* pBox = pBoxArray->pBoxes[i];
                if (!pBox)
                    continue;
                const float fScale = pPrim->lSpriteType == SPRITETYPE_ARRAY_BOX_SCALE ?
                    pBoxArray->fDirectionScale : 1.0f;
                const ZVector3 positions[4]
                {
                    pBox->vCornerBot0 + pBox->vDirectionBot0 * fScale,
                    pBox->vCornerBot1 + pBox->vDirectionBot1 * fScale,
                    pBox->vCornerTop1 + pBox->vDirectionTop1 * fScale,
                    pBox->vCornerTop0 + pBox->vDirectionTop0 * fScale
                };
                const float uv[8] { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
                for (uint32_t corner = 0; corner < 4; ++corner)
                    SetQuadVertex(pVertices[lVertex + corner], positions[corner],
                        uv[corner * 2], uv[corner * 2 + 1], pBoxArray->lColor);
                pIndices[lIndex + 0] = static_cast<uint16_t>(lVertex + 0);
                pIndices[lIndex + 1] = static_cast<uint16_t>(lVertex + 1);
                pIndices[lIndex + 2] = static_cast<uint16_t>(lVertex + 2);
                pIndices[lIndex + 3] = static_cast<uint16_t>(lVertex + 0);
                pIndices[lIndex + 4] = static_cast<uint16_t>(lVertex + 2);
                pIndices[lIndex + 5] = static_cast<uint16_t>(lVertex + 3);
                for (uint32_t corner = 0; corner < 4; ++corner)
                    pVertices[lVertex + corner].c = pBoxArray->lColor;
            }
            else if (pPrim->lSpriteType == SPRITETYPE_ARRAY_RAW)
            {
                const auto& sprite = pArrays->pSpritesRaw[i];
                for (uint32_t corner = 0; corner < 4; ++corner)
                {
                    const ZVector3 position
                    {
                        sprite.p.x + sprite.vertex[corner].x * sprite.fScale.x,
                        sprite.p.y + sprite.vertex[corner].y,
                        sprite.p.z + sprite.vertex[corner].z * sprite.fScale.y
                    };
                    SetQuadVertex(pVertices[lVertex + corner], position,
                        sprite.uv[corner * 2], sprite.uv[corner * 2 + 1], sprite.lColor[corner]);
                }
                pIndices[lIndex + 0] = static_cast<uint16_t>(lVertex + 0);
                pIndices[lIndex + 1] = static_cast<uint16_t>(lVertex + 1);
                pIndices[lIndex + 2] = static_cast<uint16_t>(lVertex + 2);
                pIndices[lIndex + 3] = static_cast<uint16_t>(lVertex + 0);
                pIndices[lIndex + 4] = static_cast<uint16_t>(lVertex + 2);
                pIndices[lIndex + 5] = static_cast<uint16_t>(lVertex + 3);
            }
            else if (pPrim->lSpriteType == SPRITETYPE_ARRAY_UV)
            {
                const auto& sprite = pArrays->pSpritesUV[i];
                const float uv[8]
                {
                    sprite.u.x, sprite.u.y, sprite.u.x, sprite.v.y,
                    sprite.v.x, sprite.v.y, sprite.v.x, sprite.u.y
                };
                WriteQuad(pVertices, pIndices, lVertex, lIndex, sprite.p,
                    sprite.fScale.x, sprite.fScale.y, 0.0f, uv, nullptr);
                for (uint32_t corner = 0; corner < 4; ++corner)
                    pVertices[lVertex + corner].c = sprite.lColor;
            }
            else
            {
                const auto& sprite = pArrays->pSprites[i];
                WriteQuad(pVertices, pIndices, lVertex, lIndex, sprite.p,
                    sprite.fScale, sprite.fScale, sprite.fAngle, nullptr, nullptr);
                for (uint32_t corner = 0; corner < 4; ++corner)
                    pVertices[lVertex + corner].c = sprite.lColor;
            }
        }
        pDynamicVB->Unlock();
        pDynamicIB->Unlock();

        g_pd3dDevice->SetStreamSource(0u, pDynamicVB->Interface(), lVertexStart * sizeof(SVertexWintel), sizeof(SVertexWintel));
        g_pd3dDevice->SetIndices(pDynamicIB->Interface());
        g_pd3dDevice->DrawIndexedPrimitive(
            D3DPT_TRIANGLELIST, static_cast<INT>(lVertexStart), 0, lVertexCount, lIndexStart, lQuadCount * 2);
    }

    ZRenderObjectInstanceSpritesArrayD3D::~ZRenderObjectInstanceSpritesArrayD3D() = default;

    void ZRenderObjectInstanceSpritesArrayD3D::Draw(ZRenderContext* pContext)
    {
        DrawSpriteArrayD3D(this, pContext);
    }
}
