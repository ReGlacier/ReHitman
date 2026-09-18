#include <Glacier/Render/Material/ZRenderBinderContextD3D.h>


namespace Glacier
{
    ZRenderBinderContextD3D::~ZRenderBinderContextD3D() = default;

    uint32_t ZRenderBinderContextD3D::GetContextType() const
    {
        return m_lContextType;
    }
}
