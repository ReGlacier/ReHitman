#include <Glacier/Render/ZPrimAccess.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Material/ZRenderMaterialSubClass.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/ZPrimAccess.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    void ZPrimAccess::Destroy()
    {
        if ((m_lStatusFlags & 2) == 0)
        {
            ZUniMemory::Delete(this);
        }
    }

    void ZPrimAccess::Lock(uint32_t lLockFlags)
    {
        ZASSERT((m_lStatusFlags & 1) == 0);

        m_lStatusFlags |= 1u;
    }

    void ZPrimAccess::Unlock()
    {
        ZASSERT((m_lStatusFlags & 1) != 0);

        m_lStatusFlags &= ~1u;
    }

    void ZPrimAccess::ReleasePrim()
    {
        // Do nothing
    }

    ZPrimAccess::~ZPrimAccess() = default;

    ZPrimAccess* ZPrimAccess::Create(const ZPrimHandle& hPrim)
    {
        ZASSERT(g_pMaterialBufferInstance);
        ZASSERT(hPrim);

        const SPrimObject* pPrimObject = hPrim;

        auto* pMaterialInstance = g_pMaterialBufferInstance->GetMaterialInstance(pPrimObject->lMaterialId);
        if (!pMaterialInstance)
        {
            return nullptr;
        }

        return pMaterialInstance->m_pMaterialSubClass->CreatePrimAccess(hPrim);
    }
}