#include <Glacier/Render/Object/ZRenderObject.h>


namespace Glacier
{
    ZRenderObject::ZRenderObject(const ZPrimHandle& hPrim, ZRenderMaterialInstance* pMaterialInstance)
        : m_pMaterialInstance(pMaterialInstance)
        , m_hPrim(hPrim)
        , m_lNumActiveInstances(0)
        , m_lFlags(0)
    {
    }

    ZRenderObject::~ZRenderObject() = default;

    void ZRenderObject::Update()
    {
        // Do nothing
    }
}