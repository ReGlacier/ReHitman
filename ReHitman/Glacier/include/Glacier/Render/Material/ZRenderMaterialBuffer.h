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
        virtual ZRenderMaterialInstance* GetMaterialInstance(uint32_t lMaterialId);
        virtual uint32_t CreateMaterialInstanceSprite(unsigned int, unsigned int, unsigned int);
        virtual uint32_t GetTextureFromMaterial(uint32_t lMaterial, const char*);
        virtual uint32_t GetAllTexturesFromMaterial(uint32_t lMaterial, uint32_t* ppTextures, uint32_t lMaxTexturesNr);
        virtual uint32_t GetTextureAnimData(uint32_t lMaterialId);

        // methods
        // members
        uint32_t m_lNumFullyOpaqueMaterials { 0u };
        uint32_t m_lNumOpaqueMaterials { 0u };
        uint32_t m_lNumTransparentMaterials { 0u };
    };

    STATIC_GLOBAL_CLASS_INSTANCE(ZRenderMaterialBuffer*, g_pMaterialBufferInstance);
}