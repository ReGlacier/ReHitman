#pragma once

#include <cstdint>

namespace Glacier
{
    class ZWINOBJ;
    struct SSpriteArray;
    struct SSpriteArrayElementUV;

    struct STextureRef
    {
        uint32_t lTextureId;
        uint32_t iSprite;
    };

    uint32_t CreateWinFaces(STextureRef* pTextures, SSpriteArrayElementUV* pSprites,
        uint32_t*& pPrims, SSpriteArray*& pSpriteArrays, uint32_t lNumSprites,
        uint32_t lDrawMode, ZWINOBJ* pOwner, uint32_t* pIconPrim = nullptr);
}
