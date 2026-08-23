#include <Glacier/ZSTL/ZOctree.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>



namespace Glacier
{
    ZOctree::ZOctree() = default;

    void ZOctree::lort()
    {}

    COctreeObj* ZOctree::AddMinMax(float*, float*, unsigned int, COctreeObj*)
    {
        // Not implemented by IOI
        ZASSERT(false);
        return nullptr;
    }

    void ZOctree::Move(COctreeObj* pObj, float*, float*)
    {
        // Not implemented by IOI
        ZASSERT(false);
    }

    void ZOctree::Delete(COctreeObj*, bool)
    {
        // Not implemented by IOI
        ZASSERT(false);
    }

    void* ZOctree::Compile(int*)
    {
        ZASSERT(false);
        return nullptr;
    }

    void ZOctree::SetScale(float fScale)
    {
        ZASSERT(fScale > 0.0f);
        m_fScale = fScale;
    }

    void ZOctree::SetOrigin(const ZVector3& vOrigin)
    {
        m_vOrigin = vOrigin;
    }

    float ZOctree::GetScale() const
    {
        return m_fScale;
    }

    void ZOctree::GetOrigin(ZVector3& vOrigin) const
    {
        vOrigin = m_vOrigin;
    }

    void ZOctree::ConvToOCS_NoAssert(int* pOCS, const ZVector3& vWorld) const
    {
        ZASSERT(m_fScale > 0.0f); // WHAT THE FUCK?!?!? YOU SAID "_NoAssert"!!!

        pOCS[0] = static_cast<int>((vWorld.x - m_vOrigin.x) * m_fScale + 32768.0f);
        pOCS[1] = static_cast<int>((vWorld.y - m_vOrigin.y) * m_fScale + 32768.0f);
        pOCS[2] = static_cast<int>((vWorld.z - m_vOrigin.z) * m_fScale + 32768.0f);
    }

    void ZOctree::ConvToOCS_NoAssert(int* pOCS, const float* pvWorld) const
    {
        ZVector3 v{pvWorld};
        ConvToOCS_NoAssert(pOCS, v);
    }
}
