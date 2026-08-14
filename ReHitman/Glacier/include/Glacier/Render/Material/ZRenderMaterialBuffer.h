#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Filesystem/IBuffer.h>
#include <Glacier/Filesystem/ZBufferReader.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderMaterialBuffer : public ZBufferReader
    {
    public:
        // vtbl
        ~ZRenderMaterialBuffer() override;
        virtual ZRenderMaterialInstance* GetMaterialInstance(uint32_t lMaterialId) = 0;
        virtual uint32_t CreateMaterialInstanceSprite(uint32_t lTextureId, uint32_t lDrawMode, uint32_t lSpriteType) = 0;
        virtual uint32_t GetTextureFromMaterial(uint32_t lMaterialId, const char* pszBinderName) = 0;
        virtual uint32_t GetAllTexturesFromMaterial(uint32_t lMaterialId, uint32_t* ppTextures, uint32_t lMaxTexturesNr) = 0;
        virtual uint32_t GetTextureAnimData(uint32_t lMaterialId) = 0;

        // methods
        ZRenderMaterialBuffer();

        // members
        uint32_t m_lNumFullyOpaqueMaterials { 0u };
        uint32_t m_lNumOpaqueMaterials { 0u };
        uint32_t m_lNumTransparentMaterials { 0u };
    };

    STATIC_GLOBAL_CLASS_INSTANCE(ZRenderMaterialBuffer*, g_pMaterialBufferInstance);
}