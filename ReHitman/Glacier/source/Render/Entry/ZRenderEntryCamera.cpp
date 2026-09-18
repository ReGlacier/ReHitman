#include <Glacier/Render/Entry/ZRenderEntryCamera.h>
#include <Glacier/Geom/ZBaseGeom.h>


namespace Glacier
{
    ZRenderEntryCamera::ZRenderEntryCamera(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryGeom(sInfo)
        , m_lDrawConForbid(0)
    {
    }

    ZRenderEntryCamera::~ZRenderEntryCamera() = default;

    void ZRenderEntryCamera::Update()
    {
        m_pBaseGeom->GetRootTM(m_ObjectToWorldMatrix.m0, m_ObjectToWorldMatrix.p0);
    }

    ZRenderEntryCamera::RENDERENTRY_BASETYPE ZRenderEntryCamera::GetType() const
    {
        return ZRenderEntryCamera::RENDERENTRY_BASETYPE::RT_CAMERA; // 2
    }
}
