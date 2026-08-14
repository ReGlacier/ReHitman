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
}