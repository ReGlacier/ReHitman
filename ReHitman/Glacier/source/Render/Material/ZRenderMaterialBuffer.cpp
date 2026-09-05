#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>


namespace Glacier
{
    ZRenderMaterialBuffer::~ZRenderMaterialBuffer() = default;
    ZRenderMaterialBuffer::ZRenderMaterialBuffer()
        : ZBufferReader()
        , m_lNumFullyOpaqueMaterials(0)
        , m_lNumOpaqueMaterials(0)
        , m_lNumTransparentMaterials(0)
    {
    }

    STATIC_CLASS_VAR_IMPL(ZRenderMaterialBuffer, ZRenderMaterialBuffer*, g_pMaterialBufferInstance, 0x008EBF18, nullptr);
}