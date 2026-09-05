#include <Glacier/GUI/ZWinFaces.h>
#include <Glacier/GUI/ZWINOBJ.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    uint32_t CreateWinFaces(STextureRef* pTextures, SSpriteArrayElementUV* pSprites,
        uint32_t*& pPrims, SSpriteArray*& pSpriteArrays, uint32_t lNumSprites,
        uint32_t lDrawMode, ZWINOBJ* pOwner, uint32_t* pIconPrim)
    {
        if (!lNumSprites)
            return 0;

        for (uint32_t i = 1; i < lNumSprites; ++i)
        {
            const STextureRef texture = pTextures[i];
            uint32_t j = i;
            while (j && pTextures[j - 1].lTextureId > texture.lTextureId)
            {
                pTextures[j] = pTextures[j - 1];
                --j;
            }
            pTextures[j] = texture;
        }

        SSpriteArrayElementUV* sortedSprites = static_cast<SSpriteArrayElementUV*>(
            ZUniMemory::Allocate(sizeof(SSpriteArrayElementUV) * lNumSprites));
        for (uint32_t i = 0; i < lNumSprites; ++i)
            sortedSprites[i] = pSprites[pTextures[i].iSprite];
        for (uint32_t i = 0; i < lNumSprites; ++i)
            pSprites[i] = sortedSprites[i];
        ZUniMemory::Free(sortedSprites);

        uint32_t arrays = 1;
        for (uint32_t i = 1; i < lNumSprites; ++i)
            arrays += pTextures[i].lTextureId != pTextures[i - 1].lTextureId;

        if (!pPrims)
        {
            pPrims = g_pRenderDll->m_pPrimControl->AllocPrimList(arrays);
            pSpriteArrays = g_pRenderDll->m_pPrimControl->AllocSpriteArrays(arrays);
        }

        uint32_t array = 0;
        uint32_t first = 0;
        for (uint32_t i = 1; i <= lNumSprites; ++i)
        {
            if (i != lNumSprites && pTextures[i].lTextureId == pTextures[first].lTextureId)
                continue;

            pPrims[array] = pOwner->CreateSpriteArray(pTextures[first].lTextureId,
                lDrawMode | 0xC000u, SPRITETYPE_ARRAY_UV);
            pSpriteArrays[array].lNumSprites = i - first;
            pSpriteArrays[array].pSpritesUV = pSprites + first;
            if (pIconPrim && *pIconPrim == pTextures[first].lTextureId)
            {
                *pIconPrim = pPrims[array];
                pIconPrim = nullptr;
            }
            ++array;
            first = i;
        }
        return arrays;
    }
}
